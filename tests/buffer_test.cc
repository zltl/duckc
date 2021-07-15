#include "duckc/buffer.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

TEST(buffer, example) {
    struct buffer* buf = buffer_new();
    ASSERT_NE(buf, nullptr);
    ASSERT_EQ(buffer_get_len(buf), 0);

    buffer_reserve_space(buf, 1024);
    ASSERT_GE(buffer_get_space_len(buf), 1024);
    ASSERT_EQ(buffer_get_len(buf), 0);

    struct buffer_slice slice;
    buffer_get_space_slice(buf, &slice);
    ASSERT_EQ(buffer_get_space_len(buf), slice.len);

    memset(slice.data, '1', slice.len);
    buffer_commit_space(buf, slice.len);
    ASSERT_EQ(buffer_get_space_len(buf), 0);
    ASSERT_EQ(buffer_get_len(buf), slice.len);

    buffer_get_data_slice(buf, 1, &slice);
    ASSERT_EQ(slice.len, buffer_get_len(buf));
    ASSERT_EQ(slice.data[10], '1');

    auto len = buffer_get_len(buf);

    const char* p = "abcdefghijk";
    size_t plen = strlen(p);
    ASSERT_EQ(plen, buffer_add_data(buf, p, plen));
    ASSERT_GE(buffer_get_len(buf), plen + len);

    buffer_get_data_slice(buf, INT64_MAX, &slice);
    ASSERT_EQ(slice.len, buffer_get_len(buf));
    ASSERT_EQ(slice.len, plen + len);

    ASSERT_EQ(slice.data[10], '1');
    printf("slice.len=%ld\n", slice.len);
    for (size_t i = 0; i < len + plen; ++i) {
        printf("%c", slice.data[i]);
    }
    printf("\n");
    ASSERT_EQ(slice.data[len], 'a');
    ASSERT_EQ(slice.data[len + 1], 'b');

    ASSERT_EQ(len + 2, buffer_drain(buf, len + 2));
    printf("buf len=%ld\n", buffer_get_len(buf));
    ASSERT_LT(buffer_get_len(buf), plen);

    buffer_get_data_slice(buf, 1000, &slice);
    ASSERT_EQ(slice.data[0], 'c');
    printf("slice.len=%ld\n", slice.len);
    for (size_t i = 0; i < slice.len; ++i) {
        printf("%c", slice.data[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
