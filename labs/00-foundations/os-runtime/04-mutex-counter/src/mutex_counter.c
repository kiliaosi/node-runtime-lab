#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum { THREAD_COUNT = 2, INCREMENTS_PER_THREAD = 100000 };

static int count = 0;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* increment_count(void* arg) {
  (void)arg;

  for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
    // TODO: Acquire count_mutex.
    pthread_mutex_lock(&count_mutex);
    count++;
    // TODO: Release count_mutex.
    pthread_mutex_unlock(&count_mutex);
  }

  return NULL;
}

int main(void) {
  pthread_t threads[THREAD_COUNT];
  size_t started = 0;

  for (; started < THREAD_COUNT; ++started) {
    int error = pthread_create(&threads[started], NULL, increment_count, NULL);
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

  pthread_mutex_destroy(&count_mutex);
  if (started != THREAD_COUNT) {
    return 1;
  }

  const int expected = THREAD_COUNT * INCREMENTS_PER_THREAD;
  printf("expected = %d, actual = %d\n", expected, count);
  return count == expected ? 0 : 1;
}
