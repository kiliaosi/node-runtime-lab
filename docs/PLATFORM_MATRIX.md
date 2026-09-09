# 双机器平台运行矩阵

## 核心规则

这门课程在公司 Rocky Linux 9 和家里原生 Windows 11 之间接力。GitHub 保存可移植的学习成果；机器相关的上游源码、构建目录和 trace 不同步。

标记含义：

- `COMMON`：代码跨平台，任选一台机器完成即可。
- `LINUX`：只要求在公司 Rocky 完成。
- `WINDOWS`：只要求在家里 Windows 完成。
- `COMPARE`：两边都运行，重点记录差异，不要求同一天完成。

## 阶段分配

| 阶段 | Rocky Linux 9 | Windows 11 原生 | 同步到 GitHub |
| --- | --- | --- | --- |
| F00–F02 | GCC/GDB/ELF 实验 | MSVC 或 ClangCL/WinDbg/PE 实验 | 公共代码、两份观测笔记 |
| F03 | 进程、fd、pipe、signal、`epoll` | HANDLE、overlapped I/O、IOCP 基础 | 概念对照笔记 |
| L01–L08 | libuv 公共 API | 同一套 libuv 公共 API | 实验代码只保留一份 |
| LS | `src/unix`、`src/linux`、`epoll` | `src/win`、IOCP | 调用链与平台差异图 |
| V / VS | 默认主要构建环境 | 可做公共 API；Windows 全量构建按需 | embedder 代码、源码笔记 |
| N | Linux Node 调用链与诊断 | Windows 专属 binding/IOCP 按需 | JS 测试、跨平台结论 |
| E | 服务端性能、故障与贡献 | Windows 兼容性验证 | 最终项目代码和报告 |

## 工具对应关系

| 问题 | Rocky Linux 9 | Windows 11 原生 |
| --- | --- | --- |
| 可执行文件格式 | `file`、`readelf`、`objdump` | `dumpbin /headers`、`dumpbin /imports` |
| 符号与调用栈 | `nm`、GDB | `dumpbin /symbols`、Visual Studio Debugger/WinDbg |
| 系统调用/I/O 观察 | `strace` | Process Monitor；更深层使用 ETW |
| CPU 性能 | `perf` | WPR + WPA（ETW） |
| 内存错误 | Valgrind、ASan | Visual Studio Diagnostic Tools、WinDbg；适用时使用 ASan |
| I/O 多路机制 | `epoll` | IOCP / overlapped I/O |
| 二进制 | ELF | PE/COFF |

这些工具不是机械的一一替代。例如 Windows 没有与 `strace` 完全等价的单一工具，课程会根据问题选择 Process Monitor、ETW 或调试器。

## 换机器时的操作

开始学习：

```text
git pull --ff-only
读取 PROGRESS.md 的“下一步”
激活本机环境
继续当前模块
```

结束学习：

```text
把结论和命令结果摘要写进 notes/
更新 PROGRESS.md
提交课程代码与笔记
git push
```

不要提交 `build/`、`sources/`、`traces/`、`.exe`、`.obj`、ELF 或完整 ETL 文件。必要的观测证据写成短文本摘要；小型截图再按需加入。
