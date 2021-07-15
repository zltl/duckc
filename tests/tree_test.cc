#include "duckc/tree.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

struct node {
  RB_ENTRY(node) entry;
  int i;
};
int intcmp(struct node *e1, struct node *e2) {
  return (e1->i < e2->i ? -1 : e1->i > e2->i);
}
RB_HEAD(inttree, node);

RB_GENERATE(inttree, node, entry, intcmp);

void print_tree(struct node *n) {
  struct node *left, *right;

  if (n == NULL) {
    printf("nil");
    return;
  }
  left = RB_LEFT(n, entry);
  right = RB_RIGHT(n, entry);
  if (left == NULL && right == NULL) {
    printf("%d", n->i);
  } else {
    printf("%d(", n->i);
    print_tree(left);
    printf(",");
    print_tree(right);
    printf(")");
  }
}

TEST(rb, example) {
  struct inttree head = RB_INITIALIZER(&head);
  int testdata[] = {20, 16, 17, 13, 3,  6,  1,  8, 2,  4,
                    10, 19, 5,  9,  12, 15, 18, 7, 11, 14};
  struct node *n;

  for (size_t i = 0; i < sizeof(testdata) / sizeof(testdata[0]); ++i) {
    if ((n = (struct node *)malloc(sizeof(struct node))) == NULL) {
      FAIL() << "EOM";
    }
    n->i = testdata[i];
    RB_INSERT(inttree, &head, n);
  }

  int tmp = -20282822;
  RB_FOREACH(n, inttree, &head) {
    ASSERT_LT(tmp, n->i);
    tmp = n->i;
    printf("%d\n", n->i);
  }
  printf("\n");

  print_tree(RB_ROOT(&head));
  printf("\n");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
