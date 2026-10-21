# 学习进度

状态约定：`[ ]` 未开始、`[~]` 正在进行、`[x]` 已通过验收。只有产物和口头检查都完成才标记通过。

## F：基础

- [x] F00 环境、编译与证据链
- [~] F01 C 最小集
- [ ] F02 C++ 最小集
- [ ] F03 Linux 运行时最小集
- [ ] F-GATE 基础闸门

## L：libuv API

- [~] L01 loop 与运行模式
- [ ] L02 handles 与 requests
- [ ] L03 loop phases 与 watcher
- [ ] L04 文件系统与线程池
- [ ] L05 stream、TCP 与背压
- [ ] L06 Pipe、TTY、UDP、DNS
- [ ] L07 进程、信号、线程与同步原语
- [ ] L08 错误、指标与资源清理
- [ ] L-GATE 独立 libuv 服务

## LS：libuv 源码

- [ ] LS01 公共模型与宏展开
- [ ] LS02 `uv_run()` 主循环
- [ ] LS03 Linux poll backend
- [ ] LS04 timer 与队列结构
- [ ] LS05 threadpool 与 fs
- [ ] LS06 TCP 完整调用链
- [ ] LS-GATE libuv trace 修改与测试

## V：V8 API

- [ ] V01 Platform、初始化与 Isolate
- [ ] V02 handles 与 GC 安全引用
- [ ] V03 Context、Realm 与脚本执行
- [ ] V04 C++ ↔ JavaScript binding
- [ ] V05 Promise、微任务与宿主职责
- [ ] V06 ArrayBuffer、BackingStore 与外部内存
- [ ] V07 GC、内存约束与 snapshot
- [ ] V08 Inspector 与完整 embedder
- [ ] V-GATE 独立 V8 shell

## VS：V8 源码

- [ ] VS01 获取、构建与源码地图
- [ ] VS02 API 入口
- [ ] VS03 对象模型与属性访问
- [ ] VS04 parser、bytecode 与 Ignition
- [ ] VS05 编译层级与反优化
- [ ] VS06 heap 与 GC
- [ ] VS07 builtins、Promise 与 microtasks
- [ ] VS-GATE V8 纵向追踪报告

## N：Node.js

- [ ] N01 启动路径与核心对象
- [ ] N02 binding 机制
- [ ] N03 Node 事件循环
- [ ] N04 `fs` 纵向切片
- [ ] N05 timers、net、stream 与 HTTP
- [ ] N06 模块系统与执行上下文
- [ ] N07 Worker、线程池与多 Isolate
- [ ] N08 async context 与可观测性
- [ ] N09 内存与性能诊断
- [ ] N10 编译、测试和修改 Node
- [ ] N-GATE Node 全链路追踪

## E：专家化项目

- [ ] E01 Native addon
- [ ] E02 可靠任务系统
- [ ] E03 mini-node
- [ ] E04 首个上游贡献

## F01 当前进度（2026-10-21）

F01 仍为进行中。按本次实际完成内容估计，主干约完成八成；这是学习范围估计，不是熟练度评分，也不是最终验收结果。详细复习记录见 [F01 C 最小集（二）](notes/F01-c-minimum-part2.md)。

- 已完成练习与代码产物：动态链表与失败清理、指针数组与显式长度、字符串/字节缓冲区、函数指针与上下文、Task/TaskNode 创建和销毁、三节点回调遍历、重复清理、头文件与分开编译、宏/条件编译、TaskStatus 枚举。
- 已完成口头检查：数组退化与 sizeof/strlen 区别、堆对象与指针变量生命周期、局部指针副本与调用者指针、static 局部变量和内部链接、static 函数作为跨模块回调、ABI 与结构体成员偏移。
- 当前设计约定：TaskNode 持有单独分配的 Task*，保留 create_task；保留头插法；单个任务创建失败时打印并跳过，成功任务继续执行并统一清理。输入 bias 为 30、20、10，头插后回调结果为 16、26、36。此任务链表按后创建先执行处理，尚未验收为完整 FIFO 队列。
- 验证证据：严格 C11 构建和主程序运行通过；临时隔离驱动验证了创建失败的三处分配路径、单节点三块内存清理、三节点九块内存清理、头指针置空及重复/空/NULL 清理。ASan/UBSan 因运行库缺失未执行；不能把分配计数检查等同于消毒器检查。
- 下次起点：实现已布置但尚未完成的 `TaskStatus push_task(TaskNode** head, Operation operation, int bias)`。校验 head 与 operation；空链表合法；复用 create_task_node，失败保留原链表；设置传入回调、头插并返回状态。随后验证不同回调的注册和执行，再完成综合验收。

## 训练日志

| 日期 | 模块 | 产物 | 卡点 | 下一步 |
| --- | --- | --- | --- | --- |
| 2026-09-09 | 初始化 | 课程骨架、环境脚本、官方资料索引；Rocky 基础工具链通过，libuv 1.52.1 两组官方测试通过 | 完整诊断工具组按阶段安装 | F00 学习与口头验收 |
| 2026-09-09 | 双环境改造 | 增加原生 Windows 11 激活、检查、源码拉取脚本及平台运行矩阵 | Windows 工具链待回家后验收 | 在任一机器开始 F00 |
| 2026-09-10 | F00 进行中 | 完成编译流水线以及 readelf、GDB、strace、perf 基础观测 | Section/Segment 保留工作级认识，后续按需回查 | F00-4 CMake/Make 增量构建，然后完成本节验收 |
| 2026-09-14 | F00 → L01 | 完成 CMake/Make 增量构建认识；增加跨平台 libuv loop 实验骨架 | 不手写 CMake，按需阅读现有配置 | 构建并预测第一个 timer/loop 程序 |
| 2026-09-14 | L01 进行中 | 手写空 loop 与一次性 timer；验证 active 状态、异步关闭、回调线程、poll 超时及 Node Timer 边界 | active、closing、referenced 仍需分开实验 | 观察 closing/ref 状态与三种 uv_run 模式 |
| 2026-09-15 | F01 进行中 | 手写一级/二级指针、结构体布局、自引用结构体、栈上链表和单个堆节点；严格构建通过 | 对齐规则修正为“按下一个成员要求放置、末尾按整体对齐补齐”；`void *` 使用前须恢复具体类型 | 实现 `create_node` 与动态链表，继续数组/字符串/缓冲区和函数指针 |
| 2026-10-21 | F01 进行中 | 补齐动态链表、缓冲区、回调/context、多文件 Task 模块、头插任务链表与 TaskStatus；严格重建和相关内存计数检查通过；完成 static 与 ABI 口头检查 | 明确释放对象不等于清空指针、局部 current 不等于调用者 head；接受头插与单项失败跳过的设计选择；独立自查仍需巩固 | 实现 push_task 注册可选择的回调，保留头插和失败跳过策略，再进行综合验收 |
