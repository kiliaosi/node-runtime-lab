# OS04 练习：两个线程领取任务

主线程已经准备好 1 到 4 号任务，再启动两个工作线程。工作线程调用 `take_task()` 领任务，从函数返回后才处理任务。本题用一行输出代表任务执行。

只实现 `take_task(int *task)`：

1. 持有 `task_mutex` 后检查 `next_task` 是否到达 `task_count`。领完时解锁并返回 `false`。
2. 有任务时，用 `*task` 将 `tasks[next_task]` 写给调用者，再推进 `next_task`。
3. 解锁后返回 `true`。每个线程有自己的局部 `task`。需要锁保护的可变状态是 `next_task`。

和[计数器练习](../04-mutex-counter/README.md)相比，这次锁要保护的是检查是否还有任务、取出任务和推进下标。任务都在启动线程之前准备完毕，所以领完就能退出。

从远端仓库根目录运行：

```bash
mkdir -p build &&
cc -std=c11 -O0 -g -Wall -Wextra -Wpedantic -Werror -pthread \
  labs/00-foundations/os-runtime/04-task-claim/src/task_claim.c \
  -o build/f03-os04-task-claim &&
timeout 5s ./build/f03-os04-task-claim
```

验收时，任务 1 到 4 各打印一次，最后显示 `expected = 4, claimed = 4` 且退出码为 0。输出顺序、每个线程分别领到多少项，由实际调度决定。脚手架的 `take_task()` 目前返回 `false`，会显示 `claimed = 0`。

下一题练习[让工作线程等待后续提交的任务](../04-task-wakeup/README.md)。
