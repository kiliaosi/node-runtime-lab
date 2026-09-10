# 学习进度

状态约定：`[ ]` 未开始、`[~]` 正在进行、`[x]` 已通过验收。只有产物和口头检查都完成才标记通过。

## F：基础

- [~] F00 环境、编译与证据链
- [ ] F01 C 最小集
- [ ] F02 C++ 最小集
- [ ] F03 Linux 运行时最小集
- [ ] F-GATE 基础闸门

## L：libuv API

- [ ] L01 loop 与运行模式
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

## 训练日志

| 日期 | 模块 | 产物 | 卡点 | 下一步 |
| --- | --- | --- | --- | --- |
| 2026-09-09 | 初始化 | 课程骨架、环境脚本、官方资料索引；Rocky 基础工具链通过，libuv 1.52.1 两组官方测试通过 | 完整诊断工具组按阶段安装 | F00 学习与口头验收 |
| 2026-09-09 | 双环境改造 | 增加原生 Windows 11 激活、检查、源码拉取脚本及平台运行矩阵 | Windows 工具链待回家后验收 | 在任一机器开始 F00 |
| 2026-09-10 | F00 进行中 | 完成编译流水线以及 readelf、GDB、strace、perf 基础观测 | Section/Segment 保留工作级认识，后续按需回查 | F00-4 CMake/Make 增量构建，然后完成本节验收 |
