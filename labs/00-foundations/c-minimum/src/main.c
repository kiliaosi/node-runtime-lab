#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "task.h"


void test_ptr(void);
void test_struct(void);
void struct_ptr(void);
void link_list(void);
void heap_node(void);
void heap_list(void);
void test_buffer();
int calculate(int num1, int num2, int (*operation)(int, int));
int add(int, int);
int calculate_with_context(int num1, int num2, Operation operation, void* context);
static int multiply_with_bias(int a, int b, void* context);



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

Node* create_node(void* data);
void destroy_list(Node** head);
void print_nodes(Node* node[], size_t size);





int main(void) {
  test_ptr();
  test_struct();
  struct_ptr();
  link_list();
  heap_node();
  heap_list();
  test_buffer();
  int sum = calculate(2, 4, add);
  printf("2 + 4 = %d\n", sum);

  int bias = 10;

  int sum_with_bias = calculate_with_context(2, 4, add_with_bias, &bias);
  printf("2 + 4 + bias(10) = %d\n", sum_with_bias);

  Task task = {.operation = add_with_bias, .context = &bias};

  int result = task.operation(2, 4, task.context);

  printf("%d", result);

  int parepare_result = prepare_task(&task, bias);

  if (parepare_result == TASK_OK) {

    task.operation(2,4, task.context);

    destroy_task(&task);
  }

  // taskNode

  TaskNode* task_node = create_task_node(bias);

  if(task_node == NULL) {
    printf("create_task_node failed\n");
    return EXIT_FAILURE;
  }

  int node_result = task_node->task->operation(2, 4, task_node->task->context);

  printf("\n node_result is: %d\n", node_result);

  destroy_task_node(&task_node);
  printf("after destroy: node == NULL: %d\n", task_node == NULL);

  destroy_task_node(&task_node);
  printf("second destroy: node == NULL: %d\n", task_node == NULL);

  // 测试


  int bias_array[] = {30, 20, 10};

  Operation operations[] = { add_with_bias, multiply_with_bias, add_with_bias };

  TaskNode* head = NULL;

  // 创建
  for(size_t i = 0; i< 3; i++) {

    TaskStatus task_status = push_stack(&head, operations[i], bias_array[i]);

    if(task_status == TASK_ERROR) {
      printf("cpush task error");
    }
  }

  // 遍历
  TaskNode* current = head;

  run_tasks(current, 2, 4);

  // 销毁
  destroy_task_list(&head);

  // 检查销毁情况
  printf("destroy task list:%d", head == NULL);

  // 二次销毁
  destroy_task_list(&head);

  // 检查销毁情况
  printf("destroy task list:%d", head == NULL);

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

  printf("(*p).x=%d,(*p).y=%d\n", (*p).x, (*p).y);
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
  Node* node = create_node("heap-node");

  if (node == NULL) {
    printf("create node failed\n");
    return;
  }

  printf("node->data=%s, node=%p\n", (char*)(node->data), (void*)node);

  free(node);

  node = NULL;
}

Node* create_node(void* data) {
  Node* node = malloc(sizeof *node);

  if (node == NULL) {
    printf("malloc failed\n");
    return NULL;
  }

  node->data = data;
  node->next = NULL;
  return node;
}

void heap_list(void) {
  Node* node1 = create_node("node1");
  Node* node2 = create_node("node2");
  Node* node3 = create_node("node3");

  Node* check_list[] = {node1, node2, node3};

  for (size_t i = 0; i < sizeof(check_list) / sizeof(check_list[0]); i++) {
    if (check_list[i] == NULL) {
      printf("create node failed\n");
      destroy_list(&node1);
      destroy_list(&node2);
      destroy_list(&node3);
      return;
    }
  }

  print_nodes(check_list, 3);

  node1->next = node2;
  node2->next = node3;

  Node* current = node1;

  // 遍历节点
  while (current != NULL) {
    printf("current.data=%s\n", (char*)current->data);
    current = current->next;
  }

  // 遍历释放
  destroy_list(&node1);
}

void destroy_list(Node** head) {
  if (*head == NULL) {
    return;
  }

  while (*head != NULL) {
    Node* next = (*head)->next;
    free(*head);
    *head = next;
  }
}

void print_nodes(Node* node[], size_t size) {
  for (size_t i = 0; i < size; i++) {
    printf("node[%zu].data=%s\n", i, (char*)node[i]->data);
  }
}

void test_buffer() {
  char raw[] = {'a', 'b', 'c', 'd', 'e'};

  fwrite(raw, 1, sizeof(raw), stdout);
  putchar('\n');
}

int add(int a, int b) { return a + b; }

int calculate(int num1, int num2, int (*operation)(int, int)) { return operation(num1, num2); }



int calculate_with_context(int num1, int num2, Operation operation, void* context) {
  return operation(num1, num2, context);
}


static int multiply_with_bias(int a, int b, void* context){
  int bias = *(int*)context;

  return a * b + bias;
}
