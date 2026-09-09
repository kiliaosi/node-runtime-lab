# 官方资料索引

课程笔记可以参考社区内容，但形成结论时优先使用当前版本的官方文档、官方仓库、规范与原始设计资料。

## 版本与 Node.js

- [Node.js Releases](https://nodejs.org/en/about/previous-releases)：确认 Current/LTS/Maintenance 状态。
- [Node v24.20.0 source](https://github.com/nodejs/node/tree/v24.20.0)：课程固定 Node 源码。
- [Node v24.20.0 BUILDING.md](https://github.com/nodejs/node/blob/v24.20.0/BUILDING.md)：工具链、debug、ASan、测试和增量构建。
- [Node v24.20.0 src/README.md](https://github.com/nodejs/node/blob/v24.20.0/src/README.md)：Isolate、Context、Environment、Realm、binding、HandleWrap/ReqWrap 等核心概念。
- [Node dependencies guide](https://github.com/nodejs/node/blob/main/doc/contributing/maintaining/maintaining-dependencies.md)：Node 如何管理 V8、libuv 等依赖。
- [Node.js API v24](https://nodejs.org/docs/latest-v24.x/api/)：与课程主版本对应的公开 API。
- [Node.js contribution guide](https://github.com/nodejs/node/blob/main/CONTRIBUTING.md)：构建、测试、提交与 review 入口。

## libuv

- [libuv v1 documentation](https://docs.libuv.org/en/v1.x/)：API 参考。
- [Design overview](https://docs.libuv.org/en/v1.x/design.html)：event loop、handles/requests、网络 I/O 与线程池模型。
- [Basics guide](https://docs.libuv.org/en/v1.x/guide/basics.html)：最小 loop 和 handle/request 入门。
- [Event loop API](https://docs.libuv.org/en/v1.x/loop.html)：`uv_run`、运行模式和 loop 配置。
- [Handle API](https://docs.libuv.org/en/v1.x/handle.html)：生命周期、ref/unref、close。
- [Request API](https://docs.libuv.org/en/v1.x/request.html)：一次性异步请求与取消。
- [Thread pool work scheduling](https://docs.libuv.org/en/v1.x/threadpool.html)：全局线程池与 `uv_queue_work`。
- [File system operations](https://docs.libuv.org/en/v1.x/fs.html)：同步/异步双形态和清理约定。
- [libuv v1.52.1 source](https://github.com/libuv/libuv/tree/v1.52.1)：课程固定源码。
- [libuv contribution guide](https://github.com/libuv/libuv/blob/v1.x/CONTRIBUTING.md)：构建、测试和贡献约定。

## V8

- [Getting started with embedding V8](https://v8.dev/docs/embed)：Isolate、handles、Context、templates、exceptions 和官方样例。
- [V8 public C++ API reference](https://v8.github.io/api/head/)：便于检索；课程结论仍以固定版本头文件为准。
- [Checking out V8](https://v8.dev/docs/source-code)：`depot_tools` 与 `fetch v8` 官方流程。
- [Building V8](https://v8.dev/docs/build)：`gclient`、GN/Ninja 与 `gm.py`。
- [V8 source](https://chromium.googlesource.com/v8/v8/)：上游官方仓库。
- [V8 blog](https://v8.dev/blog)：GC、编译管线、Sparkplug、Maglev 等原始技术文章。
- [Node v24.20.0 vendored V8 headers](https://github.com/nodejs/node/tree/v24.20.0/deps/v8/include)：Node 当前使用的精确公共头文件。
- [Node v24.20.0 vendored V8 source](https://github.com/nodejs/node/tree/v24.20.0/deps/v8)：Node 当前使用且带 Node patches 的实现。

## Linux、Windows 与工具

- [Linux man-pages project](https://www.kernel.org/doc/man-pages/)：系统调用和 libc API 的权威入口。
- [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html)：Linux readiness notification 模型。
- [GDB documentation](https://sourceware.org/gdb/documentation/)：native 调试器官方手册。
- [strace documentation](https://strace.io/)：系统调用追踪。
- [Linux perf documentation](https://perf.wiki.kernel.org/)：采样、计数器与性能分析。
- [Windows I/O Completion Ports](https://learn.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports)：IOCP 的队列、完成包和并发模型。
- [Windows Performance Recorder](https://learn.microsoft.com/en-us/windows-hardware/test/wpt/windows-performance-recorder)：基于 ETW 记录系统与应用行为。
- [Windows Performance Analyzer](https://learn.microsoft.com/en-us/windows-hardware/test/wpt/windows-performance-analyzer)：分析 WPR/ETW 产生的 ETL trace。
- [Microsoft C++ command-line tools](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line)：MSVC/Build Tools 命令行环境。
- [CMake documentation](https://cmake.org/documentation/) 与 [Ninja manual](https://ninja-build.org/manual.html)：实验项目和上游构建工具。

## 使用规则

1. 链接到 `main` 的资料只用于通用流程；研究行为时切换到课程 tag。
2. V8 在线 API 页面通常展示 `head`，遇到签名差异立即回到 `deps/v8/include`。
3. 博客里的架构图有发布日期，不能默认代表当前所有实现细节。
4. 每篇源码笔记记录 Node/libuv/V8 的 tag 或完整 commit。
