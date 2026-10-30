# 学习进度

状态约定：`[ ]` 未开始、`[~]` 正在进行、`[x]` 已通过验收。只有产物和口头检查都完成才标记通过。

## F：基础

- [x] F00 环境、编译与证据链
- [x] F01 C 最小集
- [~] F02 C++ 最小集
- [ ] F03 操作系统深入学习主线（目标已升级，尚未开始）
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

## F03 目标调整（2026-10-30）

用户明确将操作系统从最小集升级为深入掌握的长期主线。以 Linux 为主要实现与诊断对象，原生 Windows 做机制对照；课程覆盖硬件/内核边界、进程与调度、虚拟内存、并发同步、文件系统、I/O、网络、隔离与内核实现。路线和验收见 [OS_STUDY_PLAN.md](docs/OS_STUDY_PLAN.md)。

- 验收按机制理解、独立实现、故障诊断、源码修改与回归证据分别记录。
- 当前仅完成课程调整，F03 尚未开课；先完成 F02 的 test_owned_callback 和 V8 短核对，再进入 OS00。
- 保留 libuv、V8、Node 后续目标，以真实运行时问题持续深化 OS；学习周期按验收调整。

## F01 当前进度（2026-10-22）

F01 已通过本轮验收。课程要求的 C 最小集主干、综合任务链表实验及本轮口头检查已完成，可以进入 F02。该结论表示达到本阶段学习范围；独立编码、自查和稳定预测输出仍需在后续实践中巩固。验收记录见 [F01 C 最小集（三）](notes/F01-c-minimum-part3.md)，此前过程见 [第一部分](notes/F01-c-minimum-part1.md)和[第二部分](notes/F01-c-minimum-part2.md)。

- 知识范围：指针与二级指针、数组退化、字符串与字节缓冲区、struct/enum/typedef、宏与条件编译、函数指针与 void* 上下文、存储期与所有权、头文件/翻译单元/static/ABI。
- 综合产物：实现 `push_stack(TaskNode**, Operation, int)`（原计划名称为 push_task），登记指定回调并头插；实现 `run_tasks(TaskNode*, int, int)`，在 task.c 遍历执行 main.c 注册的回调，包括 main.c 内部链接的 static multiply_with_bias；创建、注册、执行与分层清理已接通。
- 设计约定：保留 TaskNode → Task → int context 的三块堆内存与 create_task；保留头插的 LIFO 顺序；单次注册失败保留原链表，调用者打印后可继续。按约定的任务链表验收，未将其描述为 FIFO 实现。当前 context 明确采用 int bias，run_tasks 借用链表与上下文，不移除节点或释放资源。
- 验证证据：严格 C11 构建和主程序运行通过；输入 bias 30/20/10、回调加/乘/加，实际遍历得到 bias 10/20/30、结果 16/28/36，两次销毁后 head 均为空。临时隔离驱动验证了空参数不分配、合法空表、登记不执行回调、头插，以及三处分配失败时原链表不变、本次失败产生的分配全部回收。此前单节点和三节点清理计数也通过。ASan/UBSan 此前因运行库缺失未执行，不能将分配计数等同于消毒器验证。
- 口头验收：解释指针副本与悬空别名、按值传参后复制到独立堆上下文、回调借用与销毁函数所有权、失败逐层回收且不影响原链表、static 函数通过地址跨模块调用、重复执行遍历不改变链表。别名置空与综合题漏算已纠正；综合题 28/16/35/18 为分析结果，与上述主程序实测结果分开记录。
- 后续巩固：声明、定义、调用保持一致；保存后核对实际编译文件；逐节点预测每次遍历；区分空指针检查与对象是否仍存活。
- 后续衔接：当日已进入 F02，并完成引用口头练习和首组生命周期/所有权代码实验；最新状态见下方 F02 当前进度。

## F02 当前进度（2026-10-30）

F02 正在进行。RAII/移动语义、智能指针、基础模板、多态、lambda 与 C 风格回调桥接已完成对应课堂练习；
V8 Maybe/空局部句柄实验与 HandleScope/持久句柄阅读已推进；double-free 和 use-after-free 均已修复并通过 Valgrind，短综合验收仍待完成。

笔记：[第一部分](notes/F02-cpp-minimum-part1.md)、[第二部分](notes/F02-cpp-minimum-part2.md)、
[第三部分](notes/F02-cpp-minimum-part3.md)、[第四部分](notes/F02-cpp-minimum-part4.md)。
实验入口见 [README](labs/00-foundations/cpp-minimum/README.md)。

- 保留成果：File 七组实验，包括 *target = std::move(*source) 的移动赋值；main.cc 的 shared_ptr、函数/类模板、抽象类多态及虚析构等实验。
- 本轮回调：无捕获 add 转函数指针，调用 2、4 得到 6；void* context 借用 int bias，先后得到 16、26。带捕获 work 在 bias=20 时创建，再通过无捕获 bridge 和 &work 调用，结果为 26。桥接代码在 test_context_callback 内，不是独立 test_lambda_bridge。
- 本轮 V8：使用 Node 24.20.0 的真实头文件完成 Maybe<int> 的 IsJust/FromMaybe/To；empty.To 失败后保留重置的 -99。test_maybe_local 使用默认空 Local<String>/MaybeLocal<String>，仅打印 ToLocal 失败返回值 0。
- 阅读范围：Local/MaybeLocal 的状态与转换；Maybe::To 和 MaybeLocal::ToLocal 的失败输出约定不同；HandleScope 管理局部句柄有效期，JS 对象由 V8 GC 管理；有效持久强句柄保活对象。没有初始化 V8 引擎、创建真实 JS 字符串或实测 GC。
- 内存故障历史：助手提供独立 memory_raii.cc 故障教材；默认浅拷贝导致同地址二次释放，Valgrind 3.26.0 已复现 Invalid free、ERROR SUMMARY 1、退出 99；释放后读取也已复现并保留修复前日志。没有泄漏不代表没有内存错误。
- 两项修复已验证：删除 Buffer 的复制构造/赋值，second 改为 Buffer& 借用；将 UAF 案例的 owner 声明放到函数外层，使 saved 最后一次读取先于 owner 析构。double-free、use-after-free 两个修复版均 ERROR SUMMARY 0、无泄漏、退出 0。当前源码已修复，错误示例作为历史材料保留。
- 本机证据：build/f02-cpp-minimum/valgrind-double-free-after-fix.log 与 valgrind-use-after-free-after-fix.log；UAF 输出 borrowed value: 42 后才出现 free 地址日志。
- 下班保存点：main.cc 末尾已建立 test_owned_callback 空函数，尚未实现、添加前置声明或接入 main，也未验证此练习。
- 续学入口：补全最后一项综合编码练习，将 unique_ptr 所有权移动、多态和 void* 回调借用串起来，要求见[第四部分笔记](notes/F02-cpp-minimum-part4.md)。实现并通过运行检查后，再简短核对 Maybe/MaybeLocal 失败处理与 Local 句柄有效期；两项通过后完成 F02，进入 F03 操作系统运行时最小集。
- 当前边界：资源错误两项修复已通过课堂检查，F02 综合验收尚未完成。ASan/UBSan 库缺失，未执行；独立自移动、分配失败注入仍未覆盖。V8 仍是包装类型与生命周期入门，没有初始化引擎或实测 GC。
- 提交记录：修复代码独立提交；本次文档记录已取得的课堂证据，不重复运行测试，F02 保持进行中。

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
| 2026-10-22 | F02 进行中 | 完成引用口头练习及 Trace、作用域/提前 return 清理、unique_ptr 移动和函数接管所有权四组实验；C++20 严格重建与输出核对通过；新增第一部分笔记 | 纠正默认空智能指针、声明顺序及调用/日志漏写；区分智能指针变量、动态对象与所有权 | 手写管理真实 C 资源的 RAII 类，明确复制约束与失败清理 |
| 2026-10-27 | F02 进行中 | 完成六组 File RAII 实验、借用、移动构造及按值参数接管；严格构建、五个文件字节和资源关闭顺序通过核对；新增第二部分笔记和可见终端任务 | 巩固对象/引用/智能指针的成员访问、显式返回值、保存后的实际源文件；右值引用和赋值运算符重载已讲解 | 实现并验证 File 移动赋值，随后继续 shared_ptr 等 F02 剩余内容 |
| 2026-10-29 | F02 进行中 | 完成 File 移动赋值、shared_ptr、函数/类模板、抽象类多态与虚析构、lambda 值/引用捕获；新增第三部分笔记 | 区分移动指针与移动对象、const 成员函数、纯虚接口及捕获时机；下班提交按用户要求不重新运行验证 | 无捕获 lambda 转函数指针，随后 C 回调/context 桥接 |
| 2026-10-30 | F02 进行中 | 完成 lambda 函数指针/context/闭包对象桥接；V8 Maybe 与空句柄实验；用 Valgrind 复现默认浅拷贝导致的 double-free | 区分 Maybe/MaybeLocal 输出约定、句柄与 GC 对象生命周期，以及无泄漏与无内存错误 | 先修 double-free，再复现并修复 use-after-free，随后综合验收 |
| 2026-10-30 | F02 资源错误修复 | 禁止 Buffer 复制并改为引用借用；UAF 的 owner 外移，保证最后一次读取早于析构；两条修复路径 Valgrind 均 0 错误、无泄漏、退出 0，保留修复前日志 | 非空借用指针仍可能悬空；引用不是新增资源所有者；无泄漏与无非法访问是不同检查 | 基于已有代码完成 F02 短综合验收，暂不标整阶段通过 |
| 2026-10-30 | F02 下班保存点 | 保存 test_owned_callback 空函数和综合练习要求，函数体、前置声明、main 调用尚待补齐；此次未编译或运行新练习 | 借用指针应在新所有者 second 存活期间使用；预期输出尚非实测结果 | 回家补全综合练习并验证，再核对 V8 失败处理与句柄生命周期，完成 F02 后进入 F03 |
| 2026-10-30 | F03 目标升级 | 将操作系统从最小集调整为深入学习主线，新增路线与分层验收，更新总纲和平台安排 | 当前只调整计划，F03 尚未开始；保持 F02 收尾任务 | 完成 test_owned_callback 与 V8 短核对后，从 OS00 进入执行与内核边界 |
