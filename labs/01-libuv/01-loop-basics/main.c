#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

static void fail_uv(const char* operation, int status) {
  fprintf(stderr, "%s failed: %s\n", operation, uv_strerror(status));
  exit(EXIT_FAILURE);
}

static void on_timer(uv_timer_t* timer) {
  puts("3. timer callback");
  uv_close((uv_handle_t*)timer, NULL);
}

int main(void) {
  uv_loop_t loop;
  uv_timer_t timer;

  int status = uv_loop_init(&loop);
  if (status < 0) {
    fail_uv("uv_loop_init", status);
  }

  status = uv_timer_init(&loop, &timer);
  if (status < 0) {
    fail_uv("uv_timer_init", status);
  }

  puts("1. before scheduling");

  status = uv_timer_start(&timer, on_timer, 300, 0);
  if (status < 0) {
    fail_uv("uv_timer_start", status);
  }

  puts("2. before uv_run");
  status = uv_run(&loop, UV_RUN_DEFAULT);
  printf("4. after uv_run: alive=%d, status=%d\n", uv_loop_alive(&loop), status);

  status = uv_loop_close(&loop);
  if (status < 0) {
    fail_uv("uv_loop_close", status);
  }

  return EXIT_SUCCESS;
}
