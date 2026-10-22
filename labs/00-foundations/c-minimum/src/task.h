#ifndef F01_TASK_H
#define F01_TASK_H
#define TASK_TRACE 0

typedef enum {
  TASK_OK = 1,
  TASK_ERROR = -1,
} TaskStatus;


typedef int (*Operation)(int, int, void*);

typedef struct {
  Operation operation;
  void* context;
} Task;

typedef struct TaskNode {
  Task* task;
  struct TaskNode* next;
} TaskNode;

int add_with_bias(int a, int b, void* context);
TaskStatus prepare_task(Task* task, int bias);
void destroy_task(Task* task);
TaskNode* create_task_node(int bias);
Task* create_task(int bias);
void destroy_task_node(TaskNode** task_node);
void destroy_task_list(TaskNode** head);
TaskStatus push_stack(TaskNode** head, Operation operation, int bias);
void run_tasks(TaskNode* node, int a, int b);

#endif
