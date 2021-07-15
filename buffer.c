#include "duckc/buffer.h"

#include <malloc.h>
#include <string.h>

#define MIN_BUFFER_SIZE 1024
#define BUFFER_CHAIN_MAX INT64_MAX
#define BUFFER_CHAIN_STRUCT_SIZE sizeof(struct buffer_chain)
#define BUFFER_SIZE_MAX INT64_MAX

static struct buffer_chain* buffer_chain_new(size_t size) {
    struct buffer_chain* chain;
    size_t to_alloc;

    if (size > BUFFER_CHAIN_MAX - BUFFER_CHAIN_STRUCT_SIZE) {
        return NULL;
    }

    // add header
    size += BUFFER_CHAIN_STRUCT_SIZE;

    if (size > BUFFER_CHAIN_STRUCT_SIZE && size < BUFFER_CHAIN_MAX / 2) {
        to_alloc = MIN_BUFFER_SIZE;
        while (to_alloc < size) {
            to_alloc <<= 1;
        }
    } else {
        to_alloc = size;
    }

    if ((chain = (struct buffer_chain*)malloc(to_alloc)) == NULL) {
        return NULL;
    }

    memset(chain, 0, BUFFER_CHAIN_STRUCT_SIZE);
    chain->capacity = to_alloc - BUFFER_CHAIN_STRUCT_SIZE;

    chain->buffer = (unsigned char*)(chain + 1);

    return chain;
}

static void buffer_chain_free(struct buffer_chain* chain) {
    if (chain->flags & BUFFER_CHAIN_FLAG_FREE) {
        free(chain->buffer);
    }
    free(chain);
}

static void buffer_chain_free_all(struct buffer_chain* chain) {
    struct buffer_chain* next;
    for (; chain; chain = next) {
        next = TAILQ_NEXT(chain, list);
        buffer_chain_free(chain);
    }
}

// Add chain to the front of buf
static void buffer_chain_insert(struct buffer* buf,
                                struct buffer_chain* chain) {
    TAILQ_INSERT_HEAD(&buf->list, chain, list);
    // buf->total_len += chain->end - chain->offset;
}

static struct buffer_chain* buffer_chain_insert_new(struct buffer* buf,
                                                    size_t datalen) {
    struct buffer_chain* chain;
    if ((chain = buffer_chain_new(datalen)) == NULL) {
        return NULL;
    }
    buffer_chain_insert(buf, chain);
    return chain;
}

static size_t buffer_chain_len(struct buffer_chain* chain) {
    return chain->end - chain->offset;
}

struct buffer* buffer_new() {
    struct buffer* buf;
    buf = (struct buffer*)malloc(sizeof(struct buffer));
    if (buf == NULL) {
        return NULL;
    }
    TAILQ_INIT(&buf->list);
    buf->total_len = 0;

    return buf;
}

void buffer_free(struct buffer* buf) {
    struct buffer_chain* chain = TAILQ_FIRST(&buf->list);
    if (chain) {
        buffer_chain_free_all(chain);
    }
    free(buf);
}

size_t buffer_get_len(const struct buffer* buf) {
    size_t res;
    res = buf->total_len;
    return res;
}

size_t buffer_get_space_len(const struct buffer* buf) {
    struct buffer_chain* chain = TAILQ_FIRST(&buf->list);
    size_t res;

    if (chain == NULL) {
        return 0;
    }

    res = chain->capacity - chain->end;

    return res;
}

size_t buffer_get_space_slice(const struct buffer* buf,
                              struct buffer_slice* slice) {
    slice->chain = TAILQ_FIRST(&buf->list);
    slice->len = 0;
    slice->data = 0;
    if (slice->chain) {
        slice->data = slice->chain->buffer + slice->chain->end;
        slice->len = slice->chain->capacity - slice->chain->end;
    }

    return slice->len;
}

void buffer_commit_space(struct buffer* buf, size_t len) {
    struct buffer_chain* chain = TAILQ_FIRST(&buf->list);
    chain->end += len;
    buf->total_len += len;
}

size_t buffer_add_data(struct buffer* buf, const void* data, size_t data_len) {
    size_t res = 0;
    struct buffer_chain* chain;
    unsigned char* pdata = (unsigned char*)data;

    if (data_len > BUFFER_SIZE_MAX - buf->total_len) {
        return res;
    }

    size_t space_len = buffer_get_len(buf);
    if (space_len) {
        size_t to_copy = space_len;
        if (to_copy > data_len) {
            to_copy = data_len;
        }
        chain = TAILQ_FIRST(&buf->list);
        chain = TAILQ_FIRST(&buf->list);
        memcpy(chain->buffer + chain->end, data, to_copy);
        chain->end += to_copy;
        buf->total_len += to_copy;
        res += to_copy;
    }

    size_t to_alloc = data_len - res;
    chain = buffer_chain_new(to_alloc);
    if (!chain) {
        return res;
    }
    memcpy(chain->buffer, pdata + res, to_alloc);
    chain->end = to_alloc;
    res += to_alloc;
    buffer_chain_insert(buf, chain);

    return res;
}

size_t buffer_add_data_own(struct buffer* buf, void* data, size_t data_len) {
    size_t res = 0;

    if (data_len > BUFFER_SIZE_MAX - buf->total_len) {
        return res;
    }

    struct buffer_chain* chain = buffer_chain_new(0);
    chain->buffer = data;
    chain->end = data_len;
    chain->flags |= BUFFER_CHAIN_FLAG_FREE;
    buffer_chain_insert(buf, chain);
    buf->total_len += data_len;

    return res;
}

// merge first ... last => chain
static void buffer_chain_merge(struct buffer* buf, struct buffer_chain* chain,
                               struct buffer_chain* first,
                               struct buffer_chain* last) {
    struct buffer_chain* cur = last;

    TAILQ_FOREACH_REVERSE_FROM(cur, &buf->list, chain_tailq_head, list) {
        size_t to_copy = cur->end - cur->offset;
        // require chain->capacity >= all to_copy+...
        memcpy(chain->buffer + chain->end, cur->buffer + cur->offset, to_copy);
        chain->end += to_copy;
        if (cur == first) {
            break;
        }
    }
}

size_t buffer_get_data_slice(struct buffer* buf, size_t min_len,
                             struct buffer_slice* slice) {
    size_t res = 0;
    slice->chain = NULL;
    slice->data = NULL;
    slice->len = 0;
    struct buffer_chain* chain = TAILQ_LAST(&buf->list, chain_tailq_head);
    if (chain == NULL) {
        return res;
    }

    size_t chainlen = buffer_chain_len(chain);
    if (chainlen >= min_len || chainlen == buf->total_len) {
        slice->chain = chain;
        slice->len = chain->end - chain->offset;
        slice->data = chain->buffer + chain->offset;
        res = slice->len;
        return res;
    }

    struct buffer_chain* last = chain;
    struct buffer_chain* first;

    size_t to_alloc = 0;
    TAILQ_FOREACH_REVERSE_FROM(chain, &buf->list, chain_tailq_head, list) {
        to_alloc += chain->end + chain->offset;
        first = chain;
        if (to_alloc >= min_len) {
            break;
        }
    }

    struct buffer_chain* new_chain = buffer_chain_new(to_alloc);
    if (!chain) {
        return 0;
    }
    buffer_chain_merge(buf, new_chain, first, last);
    buffer_chain_free_all(first);
    buf->total_len -= to_alloc;
    buffer_chain_insert(buf, new_chain);

    return 0;
}

void buffer_reserve_space(struct buffer* buf, size_t len) {
    if (buffer_get_space_len(buf) >= len) {
        return;
    }
    buffer_chain_insert_new(buf, len);
}

void buffer_move_buf(struct buffer* to, struct buffer* from) {
    struct chain_tailq_head newhead = from->list;
    // newhead:  --> f3 f2 f1 -->
    TAILQ_CONCAT(&newhead, &to->list, list);
    from->total_len = 0;
    // newhead: --> f3 f2 f1 || t3 t2 t1 -->
    to->list = newhead;
}

size_t buffer_drain(struct buffer* buf, size_t len) {
    size_t to_drop = len;

    struct buffer_chain* chain;

    while (to_drop &&
           ((chain = TAILQ_LAST(&buf->list, chain_tailq_head)) != NULL)) {
        size_t chain_size = buffer_chain_len(chain);
        if (to_drop >= chain_size) {
            printf("free\n");
            TAILQ_REMOVE(&buf->list, chain, list);
            buffer_chain_free(chain);
            to_drop -= chain_size;
        } else {
            printf("offset\n");
            chain->offset += to_drop;
            to_drop = 0;
            break;
        }
    }
    buf->total_len -= len - to_drop;
    return len - to_drop;
}
