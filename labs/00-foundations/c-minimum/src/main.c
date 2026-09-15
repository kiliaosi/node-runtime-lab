#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void test_ptr(void);
void test_struct(void);
void struct_ptr(void);
void link_list(void);
void heap_node(void);
void heap_node(void);

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  char* username;
  char* password;
  int gender;
} User;

typedef struct Node Node;

struct Node {
  void* data;
  Node* next;
};

int main(void) {
  test_ptr();
  test_struct();
  struct_ptr();
  link_list();
  heap_node();
  return EXIT_SUCCESS;
}

void test_ptr(void) {
  int x = 42, y = 99;

  int* p = &x;
  int** PP = &p;

  printf("x的值：%d\n", x);
  printf("y的值：%d\n", y);
  printf("p的值：%p\n", (void*)p);
  printf("p指向的值：%d\n", *p);
  printf("PP的值：%p\n", (void*)PP);
  printf("PP指向的值：%p\n", (void*)*PP);
  printf("PP指向的值指向的值：%d\n", **PP);

  printf("&x=%p,&y=%p\n", (void*)&x, (void*)&y);

  printf("&p=%p, PP=%p\n", (void*)&p, (void*)PP);

  *PP = &y;

  printf("x的值：%d\n", x);
  printf("y的值：%d\n", y);
  printf("p的值：%p\n", (void*)p);
  printf("p指向的值：%d\n", *p);
  printf("PP的值：%p\n", (void*)PP);
  printf("PP指向的值：%p\n", (void*)*PP);
  printf("PP指向的值指向的值：%d\n", **PP);

  **PP = 1234;

  printf("%d\n", y);
}

void test_struct(void) {
  Point point;
  point.x = 1;
  point.y = 1;

  User user;
  user.gender = 1;
  user.username = "kiliaosiasdasdasdasd";
  user.password = "123456";

  printf("point大小:%zu, user大小：%zu", sizeof(point), sizeof(user));

  // 偏移计算
  printf("point.x偏移：%zu\n", offsetof(Point, x));
  printf("point.y偏移：%zu\n", offsetof(Point, y));
  printf("user.username偏移：%zu\n", offsetof(User, username));
  printf("user.password偏移：%zu\n", offsetof(User, password));
  printf("user.gender偏移：%zu\n", offsetof(User, gender));

  typedef struct {
    // 4
    char a;
    // 4
    int b;
    // 4
    char c;
  } LayoutA;

  typedef struct {
    // 4
    int a;
    // 1
    char b;
    // 1
    char c;
    // 2
  } LayoutB;

  LayoutA a;
  LayoutB b;

  printf("LayoutA大小:%zu, LayoutB大小：%zu\n", sizeof(a), sizeof(b));

  // 计算偏移
  printf("LayoutA.a偏移：%zu\n", offsetof(LayoutA, a));
  printf("LayoutA.b偏移：%zu\n", offsetof(LayoutA, b));
  printf("LayoutA.c偏移：%zu\n", offsetof(LayoutA, c));

  printf("LayoutB.a偏移：%zu\n", offsetof(LayoutB, a));
  printf("LayoutB.b偏移：%zu\n", offsetof(LayoutB, b));
  printf("LayoutB.c偏移：%zu\n", offsetof(LayoutB, c));
}

void struct_ptr(void) {
  Point point;
  point.x = 1;
  point.y = 1;

  Point* p = &point;

  p->x = 2;
  p->y = 3;

  printf("point.x=%d, point.y=%d\n", point.x, point.y);
  printf("p->x=%d, p->y=%d\n", p->x, p->y);

  printf("p.x=%d,p.y=%d\n", (*p).x, (*p).y);
}

void link_list(void) {
  Node node1;
  node1.data = "node1";
  node1.next = NULL;

  Node node2;
  node2.data = "node2";
  node2.next = NULL;

  Node node3;
  node3.data = "node3";
  node3.next = NULL;
  node2.next = &node3;

  node1.next = &node2;

  printf("node1.data=%s, node1.next=%s\n", (char*)node1.data, (char*)node1.next->data);

  // 遍历链表

  Node* current = &node1;

  while (current != NULL) {
    printf("current.data=%s\n", (char*)current->data);
    current = current->next;
  }
}

void heap_node(void) {
  Node* node = malloc(sizeof *node);

  if (node == NULL) {
    printf("malloc failed\n");
    return;
  }

  node->data = "heap-node";
  node->next = NULL;
  printf("node->data=%s, *node=%p\n", (char*)(node->data), (void*)node);

  free(node);

  node = NULL;
}