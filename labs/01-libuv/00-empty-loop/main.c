#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

int main(void) {
  uv_loop_t loop;

  int init_flag = uv_loop_init(&loop);

  printf("初始化结果：%d\n", init_flag);

  int close_flag = uv_loop_close(&loop);

  printf("关闭结果:%d", close_flag);
  return EXIT_SUCCESS;
}
