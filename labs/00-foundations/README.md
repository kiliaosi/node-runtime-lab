# F：源码阅读前的最小基础

这一阶段不是把你送回大学重学 C/C++，而是快速建立阅读 libuv/V8/Node 所需的语言和 Linux 运行时支点。

## 第一关：工具链冒烟（COMMON）

Rocky Linux：

```bash
source scripts/activate-lab.sh
cmake -S labs/00-foundations/toolchain-smoke \
  -B build/toolchain-smoke \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/toolchain-smoke
./build/toolchain-smoke/runtime-smoke
```

Windows 11 原生：

```powershell
. .\scripts\activate-windows.ps1
cmake -S .\labs\00-foundations\toolchain-smoke `
  -B .\build\toolchain-smoke-win -A x64
cmake --build .\build\toolchain-smoke-win --config Debug
.\build\toolchain-smoke-win\Debug\runtime-smoke.exe
```

Rocky 上继续回答：

```bash
file build/toolchain-smoke/runtime-smoke
ldd build/toolchain-smoke/runtime-smoke
nm -C build/toolchain-smoke/runtime-smoke | grep -E 'main|run_callback'
strace -f -o /tmp/runtime-smoke.strace ./build/toolchain-smoke/runtime-smoke
```

1. 这是哪种架构、哪种格式的可执行文件？
2. 哪些库是运行时动态加载的？
3. `run_callback` 在符号表中叫什么？
4. 输出文本最终通过哪个系统调用进入 stdout？

Windows 上使用“Developer PowerShell for VS”回答对应问题：

```powershell
dumpbin /headers .\build\toolchain-smoke-win\Debug\runtime-smoke.exe
dumpbin /imports .\build\toolchain-smoke-win\Debug\runtime-smoke.exe
dumpbin /symbols .\build\toolchain-smoke-win\Debug\runtime-smoke.exe |
  Select-String 'main|run_callback'
```

Windows 结果应识别为 PE/COFF，不要照搬 ELF 术语。系统活动观察留到 ETW/Process Monitor 小节。

## 第二关：调试回调（平台任选）

```bash
gdb ./build/toolchain-smoke/runtime-smoke
```

在 gdb 中：

```gdb
break run_callback
run
backtrace
print task->name
print task->value
next
continue
```

要求能解释：函数指针如何进入 `run_callback`，`void* context` 为什么能恢复成 `Task*`，这个指针是否拥有 `Task`。

Windows 可在 Visual Studio 中打开生成的 `.sln`，在 `run_callback` 设置断点，观察 Call Stack 和 `context`；后续再引入 WinDbg/CDB 命令行调试。

## 第三关：读懂构建命令

```bash
cmake --build build/toolchain-smoke --verbose
```

从输出中找到：头文件搜索路径、C++ 标准、debug symbol 参数、编译步骤和链接步骤。

## 通过标准

- 能独立构建和运行。
- 能用 gdb 在回调内停下并查看上下文。
- 能从 strace 中找到写 stdout 和进程退出。
- 能准确区分 callback、context pointer 和资源所有权。
- 能解释 `.cc → .o → executable` 的变化。

完成后在 `notes/F00-toolchain.md` 写第一篇七步法笔记，并更新 `PROGRESS.md`。
