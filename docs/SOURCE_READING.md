# 源码阅读方法

## 核心原则：纵向切片，不顺序通读

一次只追一个可以运行和验证的问题。例如：

```text
fs.readFile()
  → Node 内部 JS
  → internalBinding('fs')
  → Node C++ binding / ReqWrap
  → uv_fs_read()
  → libuv threadpool
  → pread64 syscall
  → 完成通知回到 event loop
  → JS callback / Promise
```

先能复现行为，再读公开 API 契约，最后进入实现。没有实验锚点时，大型源码中的细节很难形成长期记忆。

## 每条调用链使用七步法

1. **定义问题**：只问一个问题，例如“异步文件读取在哪个线程调用 `pread`？”
2. **最小复现**：删除业务代码，只保留能触发行为的十几行程序。
3. **读契约**：查当前版本的官方 API 文档和头文件注释。
4. **找到边界**：公开入口、JS/C++ binding、平台抽象、系统调用分别在哪里。
5. **静态追踪**：`rg` 找定义、引用、类型和关联测试，画出最短路径。
6. **动态证明**：断点、backtrace、`strace -f`、trace event 或 perf 至少选一种。
7. **改变变量**：把同步改异步、扩大并发、取消请求或提前关闭资源，验证模型是否仍成立。

## 搜索顺序

以 `uv_fs_read` 为例：

```bash
git grep -n "uv_fs_read" -- include src test
git grep -n -E "uv__fs_work|uv__work_submit" -- src
git grep -n "UV_FS_READ" -- src test
```

推荐顺序：

1. 声明和注释。
2. 最接近平台无关层的实现。
3. 当前平台实现（公司读 Linux，家里读 Windows）。
4. 对应测试。
5. blame/history（只有理解当前实现后再看）。

不要一开始就追宏的每层展开或同时读所有平台实现。同一个 API 先在当前机器追通一条路径，再到另一台机器补齐 Linux epoll / Windows IOCP 对照。

## 如何记一条源码笔记

每篇 `notes/` 笔记固定回答：

```text
问题：
我的初始预测：
最小复现：
公开 API 契约：
关键对象及所有者：
线程/队列/生命周期：
调用链（文件:符号，不写易失效的死行号）：
动态证据：
失败实验：
最终模型：
仍未解决的问题：
```

源码在升级后行号会漂移，因此学习笔记以“tag + 文件 + 符号”为主；只在 code review 中使用精确行号。

## libuv 的阅读抓手

- `include/uv.h`：公共 API 和回调类型。
- `include/uv/unix.h`：Unix 平台结构字段。
- `src/unix/core.c`：loop 核心路径。
- `src/threadpool.c`：全局线程池与 work queue。
- `src/unix/fs.c`：文件系统请求。
- `src/unix/tcp.c`、stream 相关实现：网络路径。
- `test/`：最可靠的行为边界示例之一。

实际文件布局以 `v1.52.1` checkout 为准，课程中首次进入每条路径时重新用 `git grep` 确认。

## V8 的阅读抓手

- `include/`：embedder 能依赖的公共 API，优先级最高。
- `src/api/`：公共 API 进入内部对象的入口。
- `src/execution/`：Isolate 与执行状态。
- `src/handles/`：GC 安全引用。
- `src/objects/`：对象表示与属性系统。
- `src/heap/`：分配和 GC。
- `src/interpreter/`：Ignition 与字节码。
- `src/compiler/`：优化编译管线。
- `src/builtins/`：内建函数实现。

V8 内部类型、生成代码和宏很多。第一次追踪只要求找到“公共 API → 内部入口 → 核心子系统”，不要强行把每个模板和生成器同时展开。

## Node 的阅读抓手

- `lib/`：公开 JS 核心模块。
- `lib/internal/`：内部 JS 实现。
- `src/`：C++ runtime 与 bindings。
- `src/README.md`：Node 核心对象和 C++ 约定的官方入口。
- `deps/uv`、`deps/v8`：Node 使用的精确依赖源码。
- `test/`：API 行为、回归和平台差异。

第一次追 Node API 时先找 JS 入口，再找 `internalBinding()` 的名称，然后找 binding 注册和 C++ 回调；不要直接在 C++ 目录盲搜业务 API 名称。

## 判断“真的读懂”的标准

- 能预测一个边界实验的结果。
- 能说清对象由谁创建、谁持有、在哪个线程访问、何时释放。
- 能用断点或系统调用记录证明关键步骤。
- 能修改一个条件并解释行为变化。
- 能指出哪些结论来自文档，哪些来自源码，哪些只是推断。
