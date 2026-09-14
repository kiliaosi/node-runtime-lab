# L01：libuv 事件循环与定时器基础

状态：进行中

日期：2026-09-14

环境：Rocky Linux 9.8 x86_64、libuv 1.52.1、GCC Toolset 14

## 一、libuv 的核心对象模型

libuv 面向使用者的主干可以概括为：

```text
uv_loop_t    调度中心：在哪里等待和分发事件
uv_handle_t  长期资源：持续观察什么事件来源
uv_req_t     单次操作：当前提交了哪一次异步操作
callback     事件就绪或操作完成后做什么
```

`uv_loop_t` 是事件循环对象。它通常最先初始化、最后关闭，并且必须覆盖关联 handle 和 request 的生命周期。它保存循环运行所需的队列、计数器、定时器和平台后端状态，但不是整个 libuv 库的全局单例；一个程序可以拥有多个 loop。

`uv_handle_t` 是长期存在的事件来源或资源的通用基础类型。实际代码通常使用具体类型，例如 `uv_timer_t`、`uv_tcp_t`、`uv_udp_t` 和 `uv_async_t`。

`uv_req_t` 是一次具体操作的通用基础类型，例如 `uv_write_t`、`uv_connect_t`、`uv_fs_t` 和 `uv_work_t`。handle 通常需要 `uv_close()`；request 通常随一次操作完成而结束，部分 request 另有清理 API，例如 `uv_fs_req_cleanup()`。

## 二、loop 的最小生命周期

```c
int uv_loop_init(uv_loop_t* loop);
int uv_run(uv_loop_t* loop, uv_run_mode mode);
int uv_loop_close(uv_loop_t* loop);
```

`uv_loop_t loop;` 只是在栈上分配结构体内存；`uv_loop_init(&loop)` 才把它初始化成可使用的事件循环。

`uv_run(&loop, UV_RUN_DEFAULT)` 内部存在真正的循环。它会持续处理队列、等待 I/O、运行到期 timer 和关闭 handle，直到 loop 不再存活或被要求停止。空 loop 没有活动任务，因此 `uv_run()` 会立即返回 `0`。

`uv_loop_close()` 只有在关联 handle 已关闭、request 已结束时才能成功。存在未关闭或正在关闭的 handle 时返回 `UV_EBUSY`。

## 三、timer API 与状态

```c
int uv_timer_init(uv_loop_t* loop, uv_timer_t* timer);

int uv_timer_start(uv_timer_t* timer,
                   uv_timer_cb callback,
                   uint64_t timeout,
                   uint64_t repeat);
```

`uv_timer_init(&loop, &timer)` 初始化 timer，并在这一步把 timer 关联到 loop。因此 `uv_timer_start()` 的第一个参数是 `uv_timer_t*`，不是 `uv_loop_t*`。

`timeout` 是第一次触发前的等待毫秒数；`repeat` 是后续重复间隔，`0` 表示一次性 timer。

```c
int uv_is_active(const uv_handle_t* handle);
```

`uv_is_active()` 检查 handle 是否处于活动状态，不检查结构体内存是否仍然存在。实验观察到一次性 timer 的状态变化：

```text
uv_timer_init 后                 active = 0
uv_timer_start 后                active = 1
进入一次性 timer 回调时          active = 0
```

一次性 timer 到期后，libuv 会先停止它，再调用用户回调。因此进入回调时已经 inactive，但 handle 仍然存在且尚未关闭。

## 四、关闭 handle

```c
void uv_close(uv_handle_t* handle, uv_close_cb close_callback);
```

`uv_close()` 只提交关闭，不同步完成销毁。若在 `uv_run()` 返回后调用 `uv_close()`，通常还需要再次驱动 loop 才能处理 closing handle；若在 timer 回调中调用，当前这次 `uv_run()` 可以继续处理关闭工作，然后自然返回。

`uv_timer_t*` 可以转换为通用 `uv_handle_t*`：

```c
uv_close((uv_handle_t*)timer, NULL);
```

在 `main()` 中，`timer` 是结构体，所以传 `&timer`；在回调参数中，`timer` 已经是指针，不能再写 `&timer`，否则会从 `uv_timer_t*` 错成 `uv_timer_t**`。显式强制转换可能掩盖这种指针层级错误。

## 五、timer 如何等待与调用回调

`uv_timer_start()` 只保存回调、计算到期时间、把 timer 插入管理结构并标记为 active，本身不会等待到期。真正可能阻塞当前线程的是 `uv_run()` 内部的 I/O poll。

简化流程：

```text
uv_timer_start()
    ↓ 保存绝对到期时间与回调
uv_run()
    ↓ 根据最近timer计算最大等待时间
uv__io_poll(loop, timeout)
    ↓ Linux通常进入epoll等待，可被I/O提前唤醒
uv__run_timers(loop)
    ↓ 找出到期timer
timer->timer_cb(timer)
    ↓
用户的on_timer(timer)
```

libuv 使用单调时钟记录时间。最近 timer 的剩余时间是 poll 的最大等待时间；如果 I/O 提前就绪，poll 会提前返回，下一轮重新计算剩余时间。等待由内核完成，不是用户态 while 忙等。

timer 只保证到达指定时间后尽快执行，不保证精确准时。操作系统调度、长回调和其他负载都可能造成延迟。

## 六、回调、调用者与线程

应用把函数指针交给 libuv，libuv 在事件就绪后调用它，这属于控制反转。当前纯 libuv timer 实验的调用链可以简化为：

```text
main
  → uv_run
    → uv__run_timers
      → on_timer
```

timer 回调不是由额外工作线程执行：当前实验中，主线程进入 `uv_run()`，主线程在内核中等待，醒来后仍由主线程执行 `on_timer()`。

## 七、错误码

libuv API 通常以 `0` 表示成功，以负数表示错误。

```c
const char* uv_err_name(int error_code);
const char* uv_strerror(int error_code);
```

例如 `-16`：

```text
uv_err_name(-16) → EBUSY
uv_strerror(-16) → resource busy or locked
```

前者是稳定的符号名称，后者是供人阅读的错误描述。

## 八、与 Node.js Timer 的边界

Node.js 的每个 `setTimeout()` 并不一一对应一个 `uv_timer_t`。Node 在 JavaScript 层维护 Timeout 对象和到期顺序，计算最近到期时间；Node C++ 层使用每个 Environment 的原生 timer handle 调用 libuv，等待到期后由 `RunTimers` 进入 JavaScript 的 `processTimers()`，最终执行用户回调。

因此：

```text
纯libuv timer：libuv直接调用C回调on_timer
Node setTimeout：libuv负责底层唤醒，Node timer系统执行JS回调
```

## 九、当前结论

```text
uv_loop_t + uv_handle_t + uv_req_t 构成核心对象模型。
uv_timer_t 是 handle，不是 request。
uv_timer_start() 登记任务，uv_run() 驱动循环并可能阻塞等待。
uv_close() 是异步关闭，loop 负责推进关闭流程。
active、closing、内存仍存在是三个不同概念。
```

下一步：观察 `uv_is_closing()`，然后学习 `uv_ref()`、`uv_unref()`、`uv_loop_alive()` 与三种 `uv_run` 模式。
