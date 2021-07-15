#include "duckc/queue.h"

#include <malloc.h>
#include <stdio.h>

#include "gtest/gtest.h"

TEST(slist, example) {
  struct entry {
    int val;
    SLIST_ENTRY(entry) ent_list;
  };
  SLIST_HEAD(slisthead, entry);
  slisthead head;
  SLIST_INIT(&head);

  struct entry *n1, *n2, *n3, *np;

  // insert n1
  n1 = (struct entry *)malloc(sizeof(struct entry));
  n1->val = 1;
  SLIST_INSERT_HEAD(&head, n1, ent_list);

  n2 = (struct entry *)malloc(sizeof(struct entry));
  n2->val = 2;
  SLIST_INSERT_AFTER(n1, n2, ent_list);

  int v = 0;
  SLIST_FOREACH(np, &head, ent_list) {
    ASSERT_EQ(++v, np->val);
    // printf("entry: %d\n", np->val);
  }

  // O(n) remove
  SLIST_REMOVE(&head, n2, entry, ent_list);
  free(n2);

  n3 = SLIST_FIRST(&head);
  SLIST_REMOVE_HEAD(&head, ent_list);
  free(n3);

  ASSERT_TRUE(SLIST_EMPTY(&head));
}

TEST(stailq, example) {
  struct entry {
    int val;
    STAILQ_ENTRY(entry) que;
  };

  STAILQ_HEAD(stailhead, entry);
  stailhead head = STAILQ_HEAD_INITIALIZER(head);

  struct entry *n1, *n2, *np, *np_temp;

  n1 = (struct entry *)malloc(sizeof(struct entry));
  n1->val = 1;
  STAILQ_INSERT_HEAD(&head, n1, que);

  n2 = (struct entry *)malloc(sizeof(struct entry));
  n2->val = 2;
  STAILQ_INSERT_AFTER(&head, n1, n2, que);

  int a = 0;
  STAILQ_FOREACH_SAFE(np, &head, que, np_temp) {
    ASSERT_EQ(++a, np->val);
    STAILQ_REMOVE(&head, np, entry, que);
    free(np);
  }

  ASSERT_TRUE(STAILQ_EMPTY(&head));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
