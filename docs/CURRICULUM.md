# 个人课程大纲：从 libuv、V8 到 Node.js

## 课程目标与边界

这是一条面向已有 JavaScript、Node.js、数据库和后端经验开发者的运行时工程路线。重点是 API、对象生命周期、线程模型、调用链、调试和源码，而不是业务框架。

基准用时为 28 周，每周 8–12 小时。周数只是容量估计；每个阶段都设置“闸门”，达到验收标准即可提前进入下一阶段。

每个模块的产物至少包括：可运行代码、一次失败实验、一次工具观测、一篇短笔记和一条源码调用链。

课程在公司 Rocky Linux 9 与家里原生 Windows 11 间接力。`COMMON` 实验任选平台完成；Linux 的 `epoll/strace/perf` 和 Windows 的 `IOCP/WinDbg/ETW` 分别在对应机器完成，详见 [PLATFORM_MATRIX.md](PLATFORM_MATRIX.md)。

---

## F：读运行库源码所需的最小基础（2 周）

目标：不系统重学 C/C++，只补足读 libuv、V8 和 Node 源码会反复遇到的部分。

### F00 环境、编译与证据链

- 编译、汇编、目标文件、静态库、动态库、链接和装载分别发生什么。
- `gcc/g++`、Make、CMake、`nm`、`readelf`、`objdump`、`ldd`。
- debug/release、符号、优化级别、断点和 backtrace。
- `gdb`、`strace`、`perf` 各回答什么问题。

实验：编译 `toolchain-smoke`，观察 ELF、依赖库、系统调用和函数栈。

### F01 C 最小集

- 指针、二级指针、数组退化、字符串和字节缓冲区。
- `struct`、`enum`、`typedef`、宏、条件编译。
- 函数指针与回调；`void* data` 如何携带上下文。
- 栈、堆、静态存储期；`malloc/free`；所有权约定。
- 头文件、声明/定义、翻译单元、ABI 的基本含义。

实验：不用 libuv，写一个带回调注册和上下文指针的小型任务队列。

### F02 C++ 最小集

- 类、构造/析构、RAII、引用、const、移动语义。
- `unique_ptr`、`shared_ptr` 只需理解所有权差异。
- 模板、命名空间、继承、多态和虚函数的阅读能力。
- lambda 与 C 回调的桥接。
- V8 常见包装类型：`Maybe`、`MaybeLocal`、`Local<T>`。

实验：用 RAII 封装一段 C 资源，故意制造并修复 double-free 与 use-after-free。

### F03 操作系统运行时最小集

- 进程、线程、调度和上下文切换。
- 文件描述符；普通文件、socket、pipe 在 Unix API 中的共同点。
- 阻塞、非阻塞、同步、异步是四个可组合维度。
- `epoll` 的 interest list、ready list、level/edge trigger。
- signal、eventfd、pipe、自唤醒机制。
- 虚拟内存、页、mmap、用户态/内核态和系统调用。
- Windows HANDLE、overlapped I/O 与 IOCP；明确 WSL 跑到的仍是 Linux 路径。

实验：公司写一个最小 `epoll` echo server，并用 `strace -f` 观察等待与唤醒；家里先做 IOCP 观测实验，原生 IOCP 小程序放在 libuv Windows 后端阶段。

闸门：能读懂 libuv hello-world 的类型、回调和内存释放；能解释一次 C 程序如何变成可执行文件；能用 gdb 停在回调中。

---

## L：libuv API（4 周）

目标：先把 libuv 当成独立 C 库使用，建立事件循环和 OS 抽象的准确模型。

### L01 loop 与运行模式

- `uv_loop_t`、`uv_default_loop()`、`uv_loop_init()`、`uv_loop_close()`。
- `uv_run()` 与 `UV_RUN_DEFAULT / ONCE / NOWAIT`。
- `uv_loop_alive()`、stop、backend fd、backend timeout。
- `uv_run()` 不可重入意味着什么。

实验：手动驱动 loop，预测每次 `uv_run` 后的状态。

### L02 handles 与 requests

- 长生命周期 handle 与一次性 request 的根本区别。
- `uv_handle_t`、`uv_req_t` 的公共字段与类型转换。
- active、closing、referenced 三种状态不能混为一谈。
- `uv_close()` 是异步关闭；close callback 前不得释放内存。
- `.data` 指针、容器结构和生命周期管理。

实验：故意提前 free 一个 handle，再用 ASan 看到错误；随后修复。

### L03 loop phases 与 watcher

- timer、pending、idle、prepare、poll、check、close 的职责。
- `uv_timer_t`、`uv_prepare_t`、`uv_check_t`、`uv_idle_t`、`uv_async_t`。
- `uv_ref()`、`uv_unref()` 和“为什么进程还不退出”。
- `uv_now()`、时间缓存与 timer 重复调度。

实验：排列多类 watcher，记录一次迭代中的实际顺序；跨线程用 `uv_async_send()` 唤醒 loop。

### L04 文件系统与线程池

- `uv_fs_*` 的同步/异步双形态。
- `uv_fs_t` 的 result、path、statbuf 与 `uv_fs_req_cleanup()`。
- 为什么常规文件 I/O 通常由线程池完成，而 socket 不需要。
- 全局线程池、默认 4 线程、`UV_THREADPOOL_SIZE`、跨 loop 共享。
- `uv_queue_work()`、完成回调、取消的边界。

实验：并发执行慢任务、文件读取和 DNS，改变线程池大小并测量排队。

### L05 stream、TCP 与背压

- `uv_stream_t`、listen/accept、read start/stop、write/shutdown。
- `uv_tcp_t`、buffer 分配回调、read callback 的 `nread` 语义。
- `uv_write_t` 的缓冲区所有权；写请求完成前不能释放数据。
- write queue size、高水位和应用层背压。

实验：echo server + 慢客户端；记录队列增长并实现限流。

### L06 Pipe、TTY、UDP、DNS

- `uv_pipe_t` 与 IPC/句柄传递概念。
- `uv_tty_t` 的终端差异。
- `uv_udp_t` 与无连接数据报。
- `uv_getaddrinfo()` / `uv_getnameinfo()` 为什么可能争用线程池。

实验：TCP 与 UDP 版本的相同协议；并发 DNS 与文件任务的相互影响。

### L07 进程、信号、线程与同步原语

- `uv_spawn()`、stdio 配置、退出回调和进程句柄关闭。
- `uv_signal_t` 与信号处理约束。
- `uv_thread_create()`、mutex、rwlock、semaphore、barrier、once。
- loop 的线程归属；哪些 API 可跨线程调用。

实验：父进程管理子进程；工作线程通过 `uv_async_t` 把结果交回 loop。

### L08 错误、指标与资源清理

- 错误码、`uv_strerror()`、EOF 与取消。
- `uv_walk()`、loop metrics、空闲时间、活跃 handle/request。
- 自定义 allocator 与内存观测。
- 收尾顺序：停止生产新任务、关闭 handles、drain requests、关闭 loop。

实验：实现统一资源登记与优雅退出，并证明无悬挂句柄。

闸门：能不依赖 Node 写出完整 libuv TCP 服务；能解释它何时退出、在哪个线程回调、哪些工作进入线程池、每块内存由谁释放。

---

## LS：libuv 源码（3 周）

目标：不是从目录第一页顺读，而是从已掌握的 API 沿 Linux 实现追踪。

### LS01 公共模型与宏展开

- 从 `include/uv.h` 进入，读类型、回调签名和 API 契约。
- 从 `include/uv/unix.h` 看平台结构如何嵌入公共 handle/request。
- 展开 `UV_HANDLE_FIELDS`、`UV_REQ_FIELDS`，画出内存布局。
- 结合 `test/` 找每个边界条件的可执行说明。

### LS02 `uv_run()` 主循环

- 定位 `src/unix/core.c` 中的 loop 主干。
- 活跃条件、timer、pending queue、idle/prepare/check、I/O poll、close。
- 为什么回调执行后状态可能立刻变化。
- 用 gdb 条件断点跟一轮 iteration。

### LS03 平台 poll backend：Linux epoll 与 Windows IOCP

- `epoll_create1`、`epoll_ctl`、`epoll_wait/epoll_pwait`。
- watcher 注册、事件合并、fd 失效和回调分发。
- eventfd/pipe 如何唤醒阻塞中的 loop。
- 用 `strace` 对照源码中的系统调用。
- Windows `src/win` 的 IOCP、overlapped request 和完成通知路径。
- 共同的 libuv API 如何落到两套不同的操作系统语义。

实验：Rocky 用 strace 把 TCP 系统调用对回 `src/unix`/`src/linux`；Windows 用调试器与 ETW 把同一 API 对回 `src/win`。形成一张对照调用链。

### LS04 timer 与队列结构

- timer heap 的插入、取最小、重复定时器与单调时钟。
- intrusive queue/list 的阅读方法。
- pending/closing 等队列为什么避免额外分配。

### LS05 threadpool 与 fs

- 全局 worker 初始化、任务队列、条件变量和完成通知。
- `uv__work_submit()` 到 worker，再回到 loop 的双向路径。
- `uv_fs_*` 如何封装同步 syscall、提交 work、传回 result。
- 取消为何只能在特定状态成功。

### LS06 TCP 完整调用链

- `uv_tcp_init` → bind/listen → accept → read/write → close。
- stream 公共层与 TCP 平台层如何分工。
- 写队列、部分写、EAGAIN 和可写事件。

结业修改：为一个关键路径加最小可控 trace，编译 libuv、跑相关测试，解释行为变化。

闸门：拿到任意一个已学 API，能在 30 分钟内定位声明、公共实现、当前平台实现、关联测试和至少一个 OS I/O 原语；能指出另一平台实现从哪里进入。

---

## V：V8 Embedder API（5 周）

目标：先站在“把 V8 嵌入 C++ 程序”的视角理解 API，再进入引擎实现。V8 公共 API 以同版 `include/` 头文件为最终依据。

### V01 Platform、初始化与 Isolate

- ICU/startup data、`InitializePlatform()`、`Initialize()` 与释放顺序。
- `v8::Platform` 为 V8 提供哪些宿主能力。
- `Isolate` 是带独立 heap 的 VM 实例，不等于进程或 OS 线程。
- allocator、create params、enter/exit、dispose。

实验：独立 `v8-hello` 执行一段表达式，并正确清理全部资源。

### V02 handles 与 GC 安全引用

- GC 移动物体后，为什么不能持有普通裸指针。
- `HandleScope`、`Local<T>`、`Global<T>`、`EscapableHandleScope`。
- strong/weak persistent handle 与 weak callback。
- `Maybe<T>`、`MaybeLocal<T>` 强迫调用者处理失败。

实验：返回越过作用域的值；分别写出错误实现和 `Escape()` 正确实现。

### V03 Context、Realm 与脚本执行

- Isolate 和 Context 的一对多关系。
- global object、builtins、context enter/exit。
- String、Value、Object、Array、Function 的转换与检查。
- ScriptCompiler、origin、compile/run、`TryCatch` 和异常栈。

实验：同一 Isolate 创建两个 Context，证明全局对象彼此隔离。

### V04 C++ ↔ JavaScript binding

- `FunctionCallbackInfo`、返回值、参数校验与异常抛出。
- `FunctionTemplate`、`ObjectTemplate`、prototype template。
- accessor、interceptor、internal fields、`External`。
- C++ 对象与 JS wrapper 的双向生命周期问题。

实验：将一个 C++ `Counter` 类暴露为 JS 构造函数，包含方法、访问器和析构日志。

### V05 Promise、微任务与宿主职责

- Promise API、resolver、then/catch。
- microtask policy、queue、checkpoint。
- V8 负责语言微任务，宿主决定何时推进 checkpoint。
- 这与 Node 的 `process.nextTick` 不是一回事。

实验：手动与自动 microtask policy 对比，预测并验证顺序。

### V06 ArrayBuffer、BackingStore 与外部内存

- ArrayBuffer、TypedArray、BackingStore。
- 外部分配内存的所有权和 deleter。
- external memory accounting、detach、shared backing store。
- 与 Node Buffer 的后续连接点。

实验：C++ 分配一段内存给 JS 修改，再安全回收。

### V07 GC、内存约束与 snapshot

- 新生代/老生代、minor/major GC 的概念边界。
- generational、incremental、concurrent、compacting 各解决什么。
- heap statistics、near-heap-limit callback、OOM 边界。
- weak reference、embedder graph 与循环引用风险。
- startup snapshot 的目的和限制。

实验：分配压测 + GC trace + heap statistics；观察不同引用类型对存活的影响。

### V08 Inspector 与 embedder 完整程序

- Inspector protocol 的角色。
- embedder 的 platform、isolate、context、bindings、event pump、shutdown 全生命周期。
- 构建一个可执行多段脚本的小 shell，为下一阶段源码追踪提供载体。

闸门：能从零写出 V8 embedder；能解释每一个 V8 handle 的生命周期；能把 C++ 对象安全暴露给 JS；能自主处理异常与微任务。

---

## VS：V8 源码（4 周起）

目标：建立主干地图，能够沿一条行为追踪到关键层；不追求一次通读数百万行源码。

### VS01 获取、构建和源码地图

- `depot_tools`、`fetch v8`、`gclient sync`、GN、Ninja。
- release/debug、`d8`、组件构建与符号。
- 先使用 upstream `branch-heads/13.6` 做独立构建；Node 的精确行为以 `v24.20.0/deps/v8` 为准。
- 目录地图：`include`、`src/api`、`src/execution`、`src/handles`、`src/heap`、`src/objects`、`src/interpreter`、`src/compiler`、`src/builtins`。

### VS02 从 API 入口追入引擎

- 从 `v8::Script::Run`、对象属性访问或函数调用选择一条入口。
- API checked types、handle 转换、internal object。
- Isolate、Context 与 execution state 的内部连接。

### VS03 对象模型与属性访问

- tagged value、Smi、HeapObject 的阅读概念。
- Map/Hidden Class、descriptor、elements kind。
- inline cache 与单态/多态/超多态。
- 用 V8 trace 选项观察对象形态变化，而不是只背术语。

### VS04 前端与 Ignition

- scanner/parser/AST 到 bytecode 的主路径。
- bytecode generator、register machine、interpreter dispatch。
- 用 `d8 --print-bytecode` 将一段 JS 对应到字节码。

### VS05 编译层级、优化与反优化

- Ignition、Sparkplug、Maglev、TurboFan 各自位置。
- feedback、hotness、优化触发、OSR。
- speculative optimization、guards、deoptimization。
- 用 trace 证明一段代码为何优化/反优化。

### VS06 heap 与 GC 主干

- spaces、allocation、write barrier、remembered set。
- minor/major collection 主路径。
- marking、sweeping、compaction、并发和增量阶段。
- handle roots、embedder roots 如何影响存活。

### VS07 builtins、Promise 与 microtask queue

- CSA/builtins 代码的识别方式。
- Promise reaction job 如何入队。
- MicrotaskQueue checkpoint 的实现入口。
- 与 embedder 实验、之后的 Node event loop 对照。

结业追踪：任选 `Promise.then`、属性读取或函数调用，从 JS 现象追到字节码/内建/API/关键实现，形成一篇带断点和 trace 证据的报告。

闸门：能说明要查某个 V8 行为应该先进入哪个子系统；能借助搜索、测试和调试器完成调用链，而不是靠文件名猜测。

---

## N：自底向上理解 Node.js（6 周）

目标：把已经掌握的 libuv 和 V8 接回 Node，理解 Node 如何成为二者的宿主和产品化运行时。

### N01 启动路径与核心对象

- 从 `node_main.cc` 进入进程启动。
- CLI/options、初始化、`NodeMainInstance`、Isolate 创建。
- `Environment`、`Realm`、Context、Isolate、`uv_loop_t` 的关系。
- bootstrap JS 如何加载；内建 JS 如何嵌入二进制。

实验：debug build 中在 Environment 创建和 bootstrap 处停住。

### N02 binding 机制

- `internalBinding()` 与用户不可见内部 API。
- C++ binding 初始化、`FunctionCallbackInfo`、`SetMethod()`。
- `BaseObject`、`AsyncWrap`、`HandleWrap`、`ReqWrap`。
- JS 对象、C++ wrapper、libuv handle 三方生命周期。

实验：选择一个小 binding，从 JS 调用追到 C++ 返回。

### N03 Node 的事件循环

- Node 如何驱动 `uv_run()`。
- timers、poll、check、close 与 JS 回调。
- V8 microtasks、`process.nextTick`、libuv phase 的三个队列系统。
- “一次 tick”在不同上下文中为何容易被误用。

实验：构造复杂顺序案例；通过源码断点解释，而不只背输出。

### N04 `fs` 纵向切片

- `lib/fs.js` / internal JS → binding → FSReqWrap/ReqWrap → `uv_fs_*` → threadpool → syscall。
- 参数验证、错误转换、回调/Promise 包装。
- 同步 API 与异步 API 的真实分叉点。

实验：追踪一次 `readFile`，同时用 gdb 与 strace 建立时间线。

### N05 timers、net、stream 与 HTTP

- Node timers 数据结构与 libuv timer 的关系：不是每个 JS timer 一个 `uv_timer_t`。
- `net.Socket` → TCPWrap/StreamBase → `uv_tcp_t`。
- stream 背压如何最终约束 native write queue。
- llhttp 与 HTTP JS 层的边界。

实验：慢客户端导致背压，跨 JS/C++/libuv 三层定位队列。

### N06 模块系统与执行上下文

- CommonJS wrapper/cache/resolution。
- ESM loader、module job、link/evaluate。
- `vm` Context 与 Node Realm 的限制。
- 内建模块和用户模块的加载路径。

### N07 Worker、线程池与多 Isolate

- Worker 是线程 + 独立 Isolate + 独立 event loop + Environment。
- libuv 全局线程池与 Worker 的关系。
- MessagePort、structured clone、transferable。
- CPU 密集任务、I/O 任务和 worker pool 的选择。

实验：证明多个 Worker 如何竞争同一个 libuv threadpool，并与 Worker 自己的 JS 执行线程区分。

### N08 async context 与可观测性

- async_hooks、AsyncWrap、trigger/execute async id。
- AsyncLocalStorage 如何传播上下文，在哪里可能断裂。
- diagnostics_channel、trace events、inspector、report。
- active handles/requests、event loop utilization/delay。

### N09 内存与性能诊断

- JS heap、C++ heap、external memory、Buffer、allocator RSS。
- heap snapshot、CPU profile、GC trace、native stack、core dump。
- `perf`、火焰图、`strace`、诊断报告各自的证据范围。
- 区分泄漏、缓存、碎片、未归还 allocator 和正常高水位。

### N10 编译、测试、修改 Node

- `./configure && make -jN`、debug/ASan、ccache。
- 使用 `--node-builtin-modules-path` 加速修改 JS 核心模块。
- test parallel/sequential/addons/fixtures 的基本结构。
- 做一个小型可验证修改：日志、诊断字段、边界修复或文档/测试。

闸门：能独立完成一个 Node API 的全栈源码追踪；能编译和调试 Node；能解释真实故障属于 JS、Node binding、V8、libuv 还是 OS。

---

## E：专家化项目（持续进行）

### E01 N-API / native addon

- Node-API ABI 稳定层与直接 V8 API 的取舍。
- node-gyp/CMake.js、线程安全函数、async work、对象生命周期。
- 产出一个有基准、错误处理和跨版本测试的 addon。

### E02 可靠任务系统

结合过去 RabbitMQ 状态从 Ready → Resolved → Processing 的真实故障，设计：

- 数据库状态机与合法迁移。
- 先持久化 processing/outbox，再发布消息。
- publisher confirm、consumer ack、重试、幂等键、去重。
- 乐观锁/CAS 防止旧写覆盖新状态。
- 故障注入：超快消费者、重复消息、进程崩溃、网络分区。
- 用 AsyncLocalStorage 和指标把一次任务贯穿生产者、MQ、消费者与数据库。

### E03 mini-node（教学实现，不冒充完整容器/运行时）

- 嵌入 V8，创建 Isolate/Context。
- 用 libuv 驱动 timer、异步文件读取和 TCP。
- 把 C++ binding 暴露成少量 JS API。
- 显式推进 microtask checkpoint。
- 实现资源登记、错误传播和优雅退出。

这个项目会把课程两条底层主线真正汇合。

### E04 上游贡献

- 从文档/测试/小 bug 开始。
- 学会最小复现、bisect、提交规范、CI 与 review。
- 至少向 libuv 或 Node.js 提交一个质量足够的改动；V8 贡献作为更长期目标。

---

## 不采用的学习方式

- 不从 `lib/` 或 `src/` 第一行开始顺读整个仓库。
- 不把 API 名称、事件循环阶段图或 V8 编译器名称当作掌握。
- 不在没有可运行实验时讨论复杂内部实现。
- 不混用不同主版本源码、博客图和当前行为下结论。
- 不一开始完整编译 V8；先通过 libuv 和同版 Node 源码建立阅读肌肉。
