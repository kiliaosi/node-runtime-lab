# 学习进度

状态约定：`[ ]` 未开始、`[~]` 正在进行、`[x]` 已通过验收。只有产物和口头检查都完成才标记通过。

## F：基础

- [x] F00 环境、编译与证据链
- [x] F01 C 最小集
- [~] F02 C++ 最小集
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
- 后续衔接：当日已进入 F02，并完成引用口头练习和首组生命周期/所有权代码实验；最新状态见下方 F02 当前进度。

## F02 当前进度（2026-10-29）

F02 正在进行。引用、RAII、移动语义、智能指针、基础模板、继承/多态和 lambda 值/引用捕获已完成本阶段练习；完整 F02 尚未通过验收。
笔记见 [第一部分](notes/F02-cpp-minimum-part1.md)、[第二部分](notes/F02-cpp-minimum-part2.md)和[第三部分](notes/F02-cpp-minimum-part3.md)。

- 已完成代码：main.cc 的四组 Trace 实验、shared_ptr、函数模板、类模板、抽象类多态、虚析构与智能指针、lambda 及捕获；file_raii.cc 的七组 File 实验，包括移动赋值。
- 移动赋值：使用 *target = std::move(*source) 移动两个 File 对象之间的资源；两个 unique_ptr 本身保持非空。旧文件为 old\n，新文件为 before\nafter\n，输出目标有效 1、来源有效 0、写入成功 1。自移动分支已检查源码，未单独运行自移动实验。
- 智能指针与模板：shared_ptr 的复制增加共同所有权，p.reset() 后 p 为空且计数为 0，q 继续持有对象；larger 输出 7、2.5、8，ValueBox 输出 10、2.5、20。已解释 explicit、成员初始化列表和 const 成员函数。
- 多态：Operation::run 为纯虚函数；AddOperation/MultiplyOperation 提供 const override 实现；calculate(const Operation&) 输入 2、4，输出 6、8。unique_ptr<Operation> 管理 MultiplyOperation，输入 3、4 输出 12，销毁时先派生类析构、后基类析构，再输出 after owner scope。
- lambda：无捕获 add/multiply 输入 4、5，输出 9、20；num=10 时创建引用捕获和值捕获，改成 20 后先调用引用捕获、再调用值捕获，输出 29、19。捕获练习直接写在 test_lambda 中。
- 验证记录：以上结果来自课堂实际构建/运行，使用 GNU C++ 14.2.1、C++20 和严格警告选项；此前 File 文件字节与 strace 关闭记录通过。2026-10-29 下班提交按用户要求不重复编译或运行，不把本次保存描述成新的测试。
- 续学入口：刚布置无捕获 lambda 到函数指针的转换，尚未实现/验收。在 test_lambda 的 add 定义后添加 int (*callback)(int, int) = add，并输出 callback(4, 5)，预期 9。
- 后续主题：lambda 与 C 回调/context 桥接、V8 Local<T>/Maybe/MaybeLocal 阅读、故意 double-free/use-after-free 的定位修复及 F02 综合验收。
- 验证边界：未运行本阶段 sanitizer、分配失败注入或故意的 double-free/use-after-free 实验。内容覆盖估计不作为掌握程度评分。
- 终端入口：F1 → Tasks: Run Task → F02: build and run lifetime 或 F02: build and run File RAII。

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
