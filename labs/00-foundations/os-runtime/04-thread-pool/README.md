# OS04 综合实验：关闭两个工作线程的线程池

主线程启动两个工作线程，提交四个任务，然后关闭队列。工作线程要执行完已经提交的任务，再退出。

主线程只在关闭前提交四个任务，固定数组刚好能保存它们。工作线程每次领取一个任务，在解锁后执行。

`TaskQueue` 保存待领任务、读写位置、锁、条件变量和关闭状态。`WorkerContext` 是传给工作线程的参数，包含队列地址和工作线程编号。`threads` 保存由 `pthread_create()` 通过第一个参数写入的线程标识。

## 关闭逻辑

1. `close_queue()`：持有 `queue->mutex`，把 `queue->closed` 设为 `true`，调用 `pthread_cond_broadcast()` 唤醒所有等待的工作线程，再解锁。
2. `take_task()`：在等待循环内检查关闭状态。只有同时满足队列为空和队列已关闭时，才能解锁并返回 `false`。队列为空但仍开放时继续等待。队列已关闭但还有任务时继续领取。

## 在远端终端运行

从 `/root/workspace/node-runtime-lab` 执行：

```bash
cmake -S labs/00-foundations/os-runtime/04-thread-pool -B build/f03-os04-thread-pool &&
cmake --build build/f03-os04-thread-pool &&
timeout 5s ./build/f03-os04-thread-pool/f03-os04-thread-pool
```

检查两种运行方式：

- 普通运行：1 到 4 号任务各有一条 `start` 和 `done`，`both workers joined` 最后出现，退出码为 0。不同任务的日志顺序可以变化。
- 空队列关闭：运行 `timeout 5s ./build/f03-os04-thread-pool/f03-os04-thread-pool --empty`，没有任务日志，两个线程都能退出，退出码为 0。
