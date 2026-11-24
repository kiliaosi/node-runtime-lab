#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t task_ready = PTHREAD_COND_INITIALIZER;
static bool has_task = false;
static int pending_task;

static void submit_task(int value) {
  pthread_mutex_lock(&task_mutex);
  pending_task = value;
  has_task = true;
  pthread_cond_signal(&task_ready);
  pthread_mutex_unlock(&task_mutex);
}

static bool take_task(int* value) {
  pthread_mutex_lock(&task_mutex);

  // TODO: Replace this early return with a condition-variable wait.
  while (!has_task) {
    pthread_cond_wait(&task_ready, &task_mutex);
  }

  *value = pending_task;
  has_task = false;
  pthread_mutex_unlock(&task_mutex);
  return true;
}

static void* worker_main(void* arg) {
  (void)arg;
  int value;

  puts("worker: checking for a task");
  if (take_task(&value)) {
    printf("worker: received task %d\n", value);
  } else {
    puts("worker: returned without a task");
  }

  return NULL;
}

int main(void) {
  pthread_t worker;
  int error = pthread_create(&worker, NULL, worker_main, NULL);
  if (error != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(error));
    return 1;
  }

  puts("main: press Enter to submit task 42");
  fflush(stdout);
  (void)getchar();
  submit_task(42);

  error = pthread_join(worker, NULL);
  if (error != 0) {
    fprintf(stderr, "pthread_join: %s\n", strerror(error));
    return 1;
  }

  const bool consumed = !has_task;
  pthread_cond_destroy(&task_ready);
  pthread_mutex_destroy(&task_mutex);

  puts("main: worker joined");
  return consumed ? 0 : 1;
}
