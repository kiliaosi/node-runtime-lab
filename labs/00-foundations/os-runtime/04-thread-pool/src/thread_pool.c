#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

enum { WORKER_COUNT = 2, TASK_COUNT = 4 };

typedef struct {
  int id;
  long duration_ms;
} Task;

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t not_empty;
  Task tasks[TASK_COUNT];
  size_t next_read;
  size_t next_write;
  bool closed;
} TaskQueue;

typedef struct {
  TaskQueue* queue;
  int id;
} WorkerContext;

static void submit_task(TaskQueue* queue, Task task) {
  pthread_mutex_lock(&queue->mutex);
  queue->tasks[queue->next_write] = task;
  ++queue->next_write;
  pthread_cond_signal(&queue->not_empty);
  pthread_mutex_unlock(&queue->mutex);
}

static bool take_task(TaskQueue* queue, Task* task) {
  pthread_mutex_lock(&queue->mutex);
  while (queue->next_read == queue->next_write) {
    if (queue->closed) {
      pthread_mutex_unlock(&queue->mutex);
      return false;
    }
    pthread_cond_wait(&queue->not_empty, &queue->mutex);
  }

  *task = queue->tasks[queue->next_read];
  ++queue->next_read;
  pthread_mutex_unlock(&queue->mutex);
  return true;
}

static void close_queue(TaskQueue* queue) {
  pthread_mutex_lock(&queue->mutex);
  queue->closed = true;
  pthread_cond_broadcast(&queue->not_empty);

  pthread_mutex_unlock(&queue->mutex);
}

static void run_task(int worker_id, Task task) {
  printf("worker %d start task %d\n", worker_id, task.id);

  struct timespec delay = {
      .tv_sec = task.duration_ms / 1000,
      .tv_nsec = (task.duration_ms % 1000) * 1000000L,
  };
  while (nanosleep(&delay, &delay) == -1 && errno == EINTR) {
  }

  printf("worker %d done task %d\n", worker_id, task.id);
}

static void* worker_main(void* arg) {
  WorkerContext* worker = arg;
  Task task;

  while (take_task(worker->queue, &task)) {
    run_task(worker->id, task);
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  const bool empty_run = argc == 2 && strcmp(argv[1], "--empty") == 0;
  if (argc != 1 && !empty_run) {
    fprintf(stderr, "Usage: %s [--empty]\n", argv[0]);
    return 1;
  }

  TaskQueue queue = {
      .mutex = PTHREAD_MUTEX_INITIALIZER,
      .not_empty = PTHREAD_COND_INITIALIZER,
  };
  pthread_t threads[WORKER_COUNT];
  WorkerContext worker_contexts[WORKER_COUNT];

  size_t started = 0;
  for (; started < WORKER_COUNT; ++started) {
    worker_contexts[started] = (WorkerContext){.queue = &queue, .id = (int)started + 1};
    int error = pthread_create(&threads[started], NULL, worker_main, &worker_contexts[started]);
    if (error != 0) {
      fprintf(stderr, "pthread_create: %s\n", strerror(error));
      break;
    }
  }

  if (started == WORKER_COUNT && !empty_run) {
    const Task tasks[TASK_COUNT] = {
        {1, 300},
        {2, 100},
        {3, 150},
        {4, 200},
    };
    for (size_t i = 0; i < TASK_COUNT; ++i) {
      submit_task(&queue, tasks[i]);
    }
  }

  close_queue(&queue);

  for (size_t i = 0; i < started; ++i) {
    int error = pthread_join(threads[i], NULL);
    if (error != 0) {
      fprintf(stderr, "pthread_join: %s\n", strerror(error));
      return 1;
    }
  }

  pthread_cond_destroy(&queue.not_empty);
  pthread_mutex_destroy(&queue.mutex);

  if (started != WORKER_COUNT) {
    return 1;
  }

  puts("both workers joined");
  return 0;
}
