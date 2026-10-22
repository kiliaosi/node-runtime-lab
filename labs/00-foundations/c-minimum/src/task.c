#include <stdlib.h>
#include <stdio.h>
#include "task.h"

int add_with_bias(int a, int b, void* context) {
  int bias = *(int*)context;

  return a + b + bias;
}

TaskStatus prepare_task(Task* task, int bias) {
  int* heap_bias = malloc(sizeof(*heap_bias));

  if (heap_bias == NULL) {
    return TASK_ERROR;
  }

  *heap_bias = bias;

  task->context = heap_bias;

  task->operation = add_with_bias;

  #if TASK_TRACE
    printf("task prepared\n");
  #endif

  return TASK_OK;
}


void destroy_task(Task* task) {
  free(task->context);
  task->context = NULL;
  task->operation = NULL;
}

Task* create_task(int bias) {
  Task* task = malloc(sizeof(*task));

  if(task == NULL) return NULL;

  int result = prepare_task(task, bias);

  if(result == TASK_OK) return task;

  free(task);

  return NULL;
}


TaskNode* create_task_node(int bias){
  TaskNode* task_node = malloc(sizeof(*task_node));

  if(task_node == NULL) return NULL;

  Task* task = create_task(bias);

  if(task == NULL) {
    free(task_node);
    return NULL;
  }

  task_node->task = task;

  task_node->next = NULL;

  return task_node;
}

void destroy_task_node(TaskNode** task_node) {
  if(task_node == NULL || *task_node == NULL) return;

  destroy_task((*task_node)->task);

  free((*task_node)->task);

  free((*task_node));

  *task_node=NULL;
}


void destroy_task_list(TaskNode** head) {
  if(head == NULL || *head == NULL) return;

  TaskNode* current = *head;

  while(current != NULL) {
    TaskNode* next = current->next;

    destroy_task_node(&current);

    current = next;
  }

  *head = NULL;
}

TaskStatus push_stack(TaskNode** head, Operation operation, int bias) {

  if(head == NULL || operation == NULL) return TASK_ERROR;
  TaskNode* task_node = create_task_node(bias);

  if(task_node==NULL) {
    return TASK_ERROR;
  }

  task_node->task->operation = operation;

  task_node->next = *head;

  *head = task_node;

  return TASK_OK;
}

void run_tasks(TaskNode* node, int a, int b) {
  if(node == NULL) return;

  TaskNode* current = node;

  while(current != NULL) {
    int result = current->task->operation(a, b, current->task->context);

    printf("current node %d , result is %d", *(int *) current->task->context, result);

    current = current->next;
  }
}
