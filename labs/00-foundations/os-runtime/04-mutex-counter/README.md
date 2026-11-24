# OS04 入门实验：用互斥锁保护计数器

两个线程各对同一个全局变量 `count` 加一 100000 次。主线程已负责创建线程、等待线程退出和打印最终结果。

打开 `src/mutex_counter.c`，只补 `count++` 前后的两处 TODO。两个线程应使用同一把 `count_mutex`。写完后解释为什么锁要覆盖整个自增。

从远端仓库根目录运行：

```bash
mkdir -p build &&
cc -std=c11 -O0 -g -Wall -Wextra -Wpedantic -Werror -pthread \
  labs/00-foundations/os-runtime/04-mutex-counter/src/mutex_counter.c \
  -o build/f03-os04-mutex-counter &&
./build/f03-os04-mutex-counter
```

验收输出为 `expected = 200000, actual = 200000`。未补 TODO 的脚手架故意留下普通 `int` 的数据竞争，运行结果没有保证。偶尔输出 200000 也不能证明同步正确。

下一题用互斥锁保护[两个线程领取任务](../04-task-claim/README.md)，再给队列加上等待和唤醒。
