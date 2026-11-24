# OS04 练习：用条件变量等待任务

主线程先创建一个工作线程。共享任务槽开始为空。你在远端终端按回车后，主线程才提交任务 42。工作线程拿到一个任务就退出，主线程通过 `pthread_join()` 等它结束。

打开 `src/task_wakeup.c`，只改 `take_task()` 里遇到空任务槽时直接返回的分支。持有 `task_mutex` 时检查 `has_task`，没有任务就调用 `pthread_cond_wait(&task_ready, &task_mutex)`。它会在等待时释放锁，并在返回前重新获取锁。醒来后再次检查 `has_task`，有任务再走下面已有的领取逻辑。

`submit_task()` 已经负责在同一把锁内提交任务、修改 `has_task` 并发出通知。

从远端仓库根目录运行：

```bash
mkdir -p build &&
cc -std=c11 -O0 -g -Wall -Wextra -Wpedantic -Werror -pthread \
  labs/00-foundations/os-runtime/04-task-wakeup/src/task_wakeup.c \
  -o build/f03-os04-task-wakeup &&
./build/f03-os04-task-wakeup
```

验收：看到 `worker: checking for a task` 后先不要按回车，工作线程应继续等待。按回车后应依次看到 `worker: received task 42` 和 `main: worker joined`，退出码为 0。直接按回车也应正常完成，即使主线程先提交，工作线程看到 `has_task` 已经为真就能直接领取。

下一步把同样的等待逻辑接到[线程池](../04-thread-pool/README.md)，再处理任务队列的关闭。
