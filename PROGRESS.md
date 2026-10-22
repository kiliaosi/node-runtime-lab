# 学习进度

状态约定：`[ ]` 未开始、`[~]` 正在进行、`[x]` 已通过验收。只有产物和口头检查都完成才标记通过。

## F：基础

- [x] F00 环境、编译与证据链
- [x] F01 C 最小集
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

## F01 当前进度（2026-10-22）

F01 已通过本轮验收。课程要求的 C 最小集主干、综合任务链表实验及本轮口头检查已完成，可以进入 F02。该结论表示达到本阶段学习范围；独立编码、自查和稳定预测输出仍需在后续实践中巩固。验收记录见 [F01 C 最小集（三）](notes/F01-c-minimum-part3.md)，此前过程见 [第一部分](notes/F01-c-minimum-part1.md)和[第二部分](notes/F01-c-minimum-part2.md)。

- 知识范围：指针与二级指针、数组退化、字符串与字节缓冲区、struct/enum/typedef、宏与条件编译、函数指针与 void* 上下文、存储期与所有权、头文件/翻译单元/static/ABI。
- 综合产物：实现 `push_stack(TaskNode**, Operation, int)`（原计划名称为 push_task），登记指定回调并头插；实现 `run_tasks(TaskNode*, int, int)`，在 task.c 遍历执行 main.c 注册的回调，包括 main.c 内部链接的 static multiply_with_bias；创建、注册、执行与分层清理已接通。
- 设计约定：保留 TaskNode → Task → int context 的三块堆内存与 create_task；保留头插的 LIFO 顺序；单次注册失败保留原链表，调用者打印后可继续。按约定的任务链表验收，未将其描述为 FIFO 实现。当前 context 明确采用 int bias，run_tasks 借用链表与上下文，不移除节点或释放资源。
- 验证证据：严格 C11 构建和主程序运行通过；输入 bias 30/20/10、回调加/乘/加，实际遍历得到 bias 10/20/30、结果 16/28/36，两次销毁后 head 均为空。临时隔离驱动验证了空参数不分配、合法空表、登记不执行回调、头插，以及三处分配失败时原链表不变、本次失败产生的分配全部回收。此前单节点和三节点清理计数也通过。ASan/UBSan 此前因运行库缺失未执行，不能将分配计数等同于消毒器验证。
- 口头验收：解释指针副本与悬空别名、按值传参后复制到独立堆上下文、回调借用与销毁函数所有权、失败逐层回收且不影响原链表、static 函数通过地址跨模块调用、重复执行遍历不改变链表。别名置空与综合题漏算已纠正；综合题 28/16/35/18 为分析结果，与上述主程序实测结果分开记录。
- 后续巩固：声明、定义、调用保持一致；保存后核对实际编译文件；逐节点预测每次遍历；区分空指针检查与对象是否仍存活。
- 下一步：F02 C++ 最小集，从值复制与引用的区别开始，再进入对象生命周期与 RAII。引用示例已预览，尚未作答或开始正式代码实验，F02 暂保留未开始状态。

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
| 2026-10-22 | F01 通过验收 | 完成 push_stack、Operation 数组、static 乘法回调及 run_tasks；严格重建与混合回调运行通过；注册失败路径和口头综合检查通过；补齐第三部分笔记 | 纠正悬空别名及综合题漏算；声明/定义/调用一致性和独立自查仍需巩固 | 进入 F02，从值复制与引用开始，再学习生命周期与 RAII |
