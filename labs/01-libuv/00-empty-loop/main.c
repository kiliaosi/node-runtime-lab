#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

static void on_timer(uv_timer_t* timer) {
  printf("timer callback\n");

  int is_active_flag3 = uv_is_active((uv_handle_t*)timer);

  printf("定时器对象是否存活3：%d\n", is_active_flag3);

  printf("handle是否关闭1:%d\n", uv_is_closing((uv_handle_t*)timer));
  uv_close((uv_handle_t*)timer, NULL);
  printf("handle是否关闭2:%d\n", uv_is_closing((uv_handle_t*)timer));

}

int main(void) {
  uv_loop_t loop;
  uv_timer_t timer;

  int init_flag = uv_loop_init(&loop);

  int is_alive1 = uv_loop_alive(&loop);
  printf("loop是否存活1：%d\n", is_alive1);

  int init_timer_flag = uv_timer_init(&loop, &timer);

  int is_alive2 = uv_loop_alive(&loop);
  printf("loop是否存活2：%d\n", is_alive2);
  
  int is_active_flag = uv_is_active((uv_handle_t*)&timer);

  printf("定时器对象是否存活1：%d\n", is_active_flag);
  printf("初始化结果：%d\n", init_flag);
  printf("timer初始化结果:%d\n", init_timer_flag);

  int timer_start_flag = uv_timer_start(&timer, on_timer, 1000, 0);
  int is_active_flag2 = uv_is_active((uv_handle_t*)&timer);

  int is_alive3 = uv_loop_alive(&loop);
  printf("loop是否存活3：%d\n", is_alive3);

  printf("定时器启动结果：%d\n", timer_start_flag);
  printf("定时器对象是否存活2：%d\n", is_active_flag2);
  printf("进入uv_run前\n");

  int run_flag = uv_run(&loop, UV_RUN_DEFAULT);

  printf("uv_run后：%d\n", run_flag);
  int is_alive4 = uv_loop_alive(&loop);
  printf("loop是否存活4：%d\n", is_alive4);
  

  int close_flag = uv_loop_close(&loop);

  printf("关闭结果:%d\n", close_flag);

  if (close_flag < 0) {
    printf("错误信息：%s,错误码：%s\n",
           uv_strerror(close_flag),
           uv_err_name(close_flag));
  }

  return EXIT_SUCCESS;
}



// uv第一天的复习

void test() {

}