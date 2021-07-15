#ifndef DUCKC_BUFFER_H_
#define DUCKC_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A memory chunk in buffer */
struct buffer_chain {
    // list
    TAILQ_ENTRY(buffer_chain) list;

    // capacity
    size_t capacity;

    // unuse space at the beginning of buffer
    size_t offset;

    // the position just after the last of buffer.
    size_t end;

    unsigned int flags;

// need to free buffer
#define BUFFER_CHAIN_FLAG_FREE 1

    // point to the read-write memory
    unsigned char* buffer;
};

TAILQ_HEAD(chain_tailq_head, buffer_chain);

/** buffer of unsigned char */
struct buffer {
    // list of chain
    // front --> chubnk chunk ... --> tail
    struct chain_tailq_head list;

    // total amount of bytes stored
    size_t total_len;
};

/** reference to slice of data in buffer */
struct buffer_slice {
    unsigned char* data;
    size_t len;
    struct buffer_chain* chain;
};

/** create and return a buffer */
struct buffer* buffer_new();

/** clear buffer and free.
    @param buf pointer to buffer object.
 */
void buffer_free(struct buffer* buf);

/** reserve at least 'len' bytes of spaces, in one chunk.
    @param buf pointer to a buffer object.
    @len number of bytes of spaces that 'buf' should reserver.
 */
void buffer_reserve_space(struct buffer* buf, size_t len);

/** get total number of bytes that buffer stored.
    @param buf pointer to a buffer object.
    @return number of bytes that 'buf' stored.
 */
size_t buffer_get_len(const struct buffer* buf);

/** get number space that 'buf' has.
    @param buf pointer to a buffer object.
    @return space size.
 */
size_t buffer_get_space_len(const struct buffer* buf);

/** get space slice from buf, than caller can write data into slice,
    and call 'buffer_commit_space' to store data writen.
    @param buf pointer to a buffer object.
    @slice output slice of available space.
    @return size of space.

    !NOTE: must call 'buffer_commit_space' after writing data to slice.
 */
size_t buffer_get_space_slice(const struct buffer* buf,
                              struct buffer_slice* slice);
/** notice buf to store data specfy by slice from 'buffer_get_space_slice'.
    @param buf pointer to a buffer object.
    @len data had written to slice.
 */
void buffer_commit_space(struct buffer* buf, size_t len);

/** copy data to buf.
    @param buf pointer to a buffer object.
    @data pointer to the first bytes of data.
    @data_len bytes number of data.
    @return the then number of byte that copyed.

    'buf' does not own 'data' after calling, caller should free 'data' if need.
 */
size_t buffer_add_data(struct buffer* buf, const void* data, size_t data_len);

/** store data to buf.
    @param buf pointer to a buffer object.
    @data pointer to the first bytes of data.
    @data_len bytes number of data.
    @return the then number of byte that copyed.

    !NOTE: 'buf' own the memory of 'data' after calling. 'buf' will free 'data'
   if not use it anymore.
 */
size_t buffer_add_data_own(struct buffer* buf, void* data, size_t data_len);

/** get at least 'min_len' of data from buf by slice.

    if buf->total_len < min_len, return min_len.

    @param buf pointer to a buffer object.
    @min_len minimum size of data aquire.
    @slice output slice of data.
    @return size of data that slice referenced to.
 */
size_t buffer_get_data_slice(struct buffer* buf, size_t min_len,
                             struct buffer_slice* slice);

/** move data stored by 'from' to 'to'.

    'from' whill be empty after calling.

    @to destination of data.
    @from source buf of data.
 */
void buffer_move_buf(struct buffer* to, struct buffer* from);

/** drop 'len' bytes of data from 'buf',
    If buf->total_len < len, only buf->total_len bytes will be dropped.

    @param buf pointer to a buffer object.
    @len number of bytes to drop.
    @return really droped bytes.
 */
size_t buffer_drain(struct buffer* buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // DUCKC_BUFFER_H_
