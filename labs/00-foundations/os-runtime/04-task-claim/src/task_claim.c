#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum { THREAD_COUNT = 2 };

static const int tasks[] = {1, 2, 3, 4};
static const size_t task_count = sizeof tasks / sizeof tasks[0];
static size_t next_task = 0;
static pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool take_task(int* task) {
  // TODO: Claim the next task under task_mutex.
  pthread_mutex_lock(&task_mutex);
  if (next_task < task_count) {
    *task = tasks[next_task];
    next_task++;
    pthread_mutex_unlock(&task_mutex);
    return true;
  }
  pthread_mutex_unlock(&task_mutex);

  (void)task;
  return false;
}

static void* worker_main(void* arg) {
  const int worker_id = *(const int*)arg;
  int task;

  while (take_task(&task)) {
    printf("worker %d: task %d\n", worker_id, task);
  }

  return NULL;
}

int main(void) {
  pthread_t threads[THREAD_COUNT];
  int worker_ids[THREAD_COUNT] = {1, 2};
  size_t started = 0;

  for (; started < THREAD_COUNT; ++started) {
    int error = pthread_create(&threads[started], NULL, worker_main, &worker_ids[started]);
    if (error != 0) {
      fprintf(stderr, "pthread_create: %s\n", strerror(error));
      break;
    }
  }

  for (size_t i = 0; i < started; ++i) {
    int error = pthread_join(threads[i], NULL);
    if (error != 0) {
      fprintf(stderr, "pthread_join: %s\n", strerror(error));
      return 1;
    }
  }

  pthread_mutex_destroy(&task_mutex);
  if (started != THREAD_COUNT) {
    return 1;
  }

  printf("expected = %zu, claimed = %zu\n", task_count, next_task);
  return next_task == task_count ? 0 : 1;
}
