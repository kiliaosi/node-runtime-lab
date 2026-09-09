# 环境搭建与版本策略

## 当前机器分工

```text
公司：Rocky Linux 9.8 / <ROCKY_SSH_HOST>
  ~/workspace/node-runtime-lab        Linux 上的课程仓库
  /data/node-runtime-lab/sources      libuv、Node、后续独立 V8 源码
  /data/node-runtime-lab/build        大型编译产物
  /data/node-runtime-lab/traces       perf、strace、诊断输出
  /backup/node-runtime-lab            需要保留的大型结果（按需）

家里：Windows 11 x64（原生，不依赖 WSL）
  C:\workspace\node-runtime-lab       Windows 上的课程仓库
  <NODE_RUNTIME_LAB_HOME>\sources      Windows 上游源码
  <NODE_RUNTIME_LAB_HOME>\build        Windows 编译产物
  <NODE_RUNTIME_LAB_HOME>\traces       WinDbg、ETW 等诊断输出
```

GitHub 只同步课程代码、笔记与 `PROGRESS.md`。上游源码和构建产物既不互相复制，也不提交到本仓库；两个系统各自重新生成。这能避免 ELF/PE、GCC/MSVC、路径和换行符产物互相污染。

详细的实验分配见 [PLATFORM_MATRIX.md](PLATFORM_MATRIX.md)。

## 为什么固定版本

服务器当前官方 Node 二进制报告：

```text
Node 24.20.0
libuv 1.52.1
V8 13.6.233.17-node.53
```

因此：

- 独立 libuv 实验 checkout `v1.52.1`。
- Node 源码 checkout `v24.20.0`。
- 阅读 V8 时先看 Node 源码中的 `deps/v8`，这是行为最精确的版本。
- 独立编译 V8 时使用 upstream `branch-heads/13.6`。Node 的 V8 带有 Node 自己的 patch level，所以独立 upstream checkout 用于学习 embedder/build，具体 Node 行为仍回到 `deps/v8` 核对。

版本升级单独开分支做 diff，不悄悄替换课程基线。

## 安装 Rocky 9 工具链

基础脚本会安装 GCC Toolset 14、Make、CMake、GDB、strace、perf、Valgrind 以及常用构建依赖：

```bash
cd ~/workspace/node-runtime-lab
bash scripts/bootstrap-rocky9.sh
```

进入大型源码索引和独立 V8 构建前再装完整工具组（Clang/clangd、LLVM、LLD）。公司的镜像下载速度较慢，所以不把这组大包塞进首次启动：

```bash
bash scripts/bootstrap-rocky9.sh --full
```

Node `v24.20.0` 的官方构建文档要求 GCC/G++ 12.2+、GNU Make 3.81+、受支持的 Python，并建议 4 个并行任务至少准备 8 GiB 内存。Rocky 9 系统 GCC 可能低于要求，所以课程显式使用 `gcc-toolset-14`，不替换系统编译器。

脚本还会创建无特权系统用户 `node-lab`。libuv 官方测试明确拒绝以 root 身份运行；云机仍可用 root 维护，但测试由 `node-lab` 执行。

每次新开终端先加载课程环境：

```bash
cd ~/workspace/node-runtime-lab
source scripts/activate-lab.sh
```

随后验证：

```bash
bash scripts/check-env.sh
```

## 安装原生 Windows 11 工具链

基础阶段需要：

- Git for Windows。
- CMake（确保 `cmake` 在 `PATH` 中）。
- Node.js `v24.20.0` x64，与 Rocky 的课程基线一致。
- Visual Studio 2022/Build Tools，选择“使用 C++ 的桌面开发”和 Windows 10/11 SDK。

进入 Node 源码编译前再确认安装两个组件：

- C++ Clang Compiler for Windows。
- MSBuild support for LLVM (`clang-cl`) toolset。

Node `v24.20.0` 的官方构建说明要求 Windows 10/Server 2016 以上，支持 Visual Studio 2022 或 2026；从 Node 24 开始，Windows 源码构建要求 Visual Studio 自带的 ClangCL。完整 Node 构建还需要 Python、Git for Windows 和 NASM。

打开 PowerShell 后激活课程目录：

```powershell
cd C:\workspace\node-runtime-lab
. .\scripts\activate-windows.ps1
.\scripts\check-windows.ps1
```

默认数据目录优先使用 `D:\node-runtime-lab-data`；没有 D 盘时使用 `%LOCALAPPDATA%\node-runtime-lab-data`。如需自定义，在激活前设置：

```powershell
$env:NODE_RUNTIME_LAB_HOME = 'E:\node-runtime-lab-data'
. .\scripts\activate-windows.ps1
```

脚本只设置当前 PowerShell 会话，不永久修改系统环境变量。

## 拉取版本一致的源码

```bash
bash scripts/fetch-sources.sh libuv
# 进入 Node 阶段后：
bash scripts/fetch-sources.sh node
```

该命令获取：

- `libuv` 参数获取 `/data/node-runtime-lab/sources/libuv`：独立 `v1.52.1`。
- `node` 参数获取 `/data/node-runtime-lab/sources/node`：`v24.20.0`，其中包含课程基线 V8 源码。
- `all` 参数一次获取两者。公司的 GitHub 链路较慢，推荐按阶段获取。

两个 checkout 默认是浅克隆，足以阅读、编译和打断点。需要 `git blame` 或研究历史时，再在对应目录执行：

```bash
git fetch --unshallow
```

Windows 使用对应的 PowerShell 脚本：

```powershell
. .\scripts\activate-windows.ps1
.\scripts\fetch-sources.ps1 libuv
# 进入 Node 阶段后：
.\scripts\fetch-sources.ps1 node
```

Windows 拉取 Node 时会启用 `core.symlinks=true`。如果 Git 提示无权创建符号链接，启用 Windows“开发者模式”后重新 clone。

## 编译 libuv

```bash
source scripts/activate-lab.sh
cmake -S "$NODE_RUNTIME_SOURCES/libuv" \
  -B "$NODE_RUNTIME_BUILD/libuv-debug" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON
cmake --build "$NODE_RUNTIME_BUILD/libuv-debug"
bash scripts/test-libuv.sh
```

后续会再做一份 ASan/UBSan build，不要用 sanitizers 的性能数据与 release build 对比。

Windows 原生构建：

```powershell
. .\scripts\activate-windows.ps1
cmake -S "$env:NODE_RUNTIME_SOURCES\libuv" `
  -B "$env:NODE_RUNTIME_BUILD\libuv-debug-win" `
  -A x64 -DBUILD_TESTING=ON
cmake --build "$env:NODE_RUNTIME_BUILD\libuv-debug-win" --config Debug
ctest --test-dir "$env:NODE_RUNTIME_BUILD\libuv-debug-win" `
  -C Debug --output-on-failure
```

## 编译 Node.js

进入 Node 阶段再执行，第一次完整构建可能较久：

```bash
source scripts/activate-lab.sh
cd "$NODE_RUNTIME_SOURCES/node"
export CC=gcc
export CXX=g++
./configure --debug
make -j8
```

频繁修改 Node 的 JavaScript 核心模块时，会使用官方提供的 `--node-builtin-modules-path` 方案从磁盘加载，减少反复嵌入 JS 资源造成的重编译。

Windows 完整源码构建在“x64 Native Tools Command Prompt for VS”中执行：

```bat
cd /d <NODE_RUNTIME_SOURCES>\node
vcbuild.bat debug
```

Windows 与 Linux 产物不能共用。课程默认把完整 Node/V8 高频编译留给空间和工具更充足的机器；Windows 端只在研究 Windows binding、IOCP 或复现 Windows 特有问题时做完整构建。

## 独立 V8 环境为什么延后

V8 的官方获取流程不是普通的 `git clone`：需要 Chromium `depot_tools`、`fetch v8`、`gclient sync`，构建使用 GN/Ninja；checkout 和产物也明显更大。`depot_tools` 会在该阶段提供配套构建工具。课程在 V01 前单独执行这一步，避免 F/L 阶段先花大量时间处理无关构建问题。

届时的目录约定：

```text
/data/node-runtime-lab/tools/depot_tools
/data/node-runtime-lab/sources/v8
/data/node-runtime-lab/build/v8-*
```

独立 V8 以官方 `branch-heads/13.6` 为学习基线；公共 API 最终以该 checkout 的 `include/` 为准。Node 专属行为以 Node `v24.20.0/deps/v8` 为准。

V8 也支持 Windows，但官方明确要求 Windows 上初始化/更新 `depot_tools` 时使用 `cmd.exe`，不是 PowerShell。课程到 V8 阶段再配置，不在基础阶段提前安装几十 GB 的依赖。

## VS Code

推荐扩展已写入 `.vscode/extensions.json`。安装完整工具组后启用 clangd：

- C/C++：首次阶段的代码补全与调试适配器。
- clangd：安装完整工具组后用于大型 C/C++ 源码索引、跳转和引用查找；届时在 VS Code 中关闭 C/C++ 扩展的 IntelliSense，保留其调试器。
- CMake Tools / Makefile Tools：小实验和上游工程辅助。

公司打开 Linux 上的 `~/workspace/node-runtime-lab`，扩展安装在 SSH 远端；家里直接打开 Windows 工作副本。仓库不再固定 CMake generator，由当前平台选择 Unix Makefiles 或 Visual Studio generator。大型 Node/V8 源码可单独 `File → Open Folder`，避免一个窗口同时索引多个巨型源码树。

对于 CMake 实验，生成编译数据库：

```bash
cmake -S <source> -B <build> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sfn <build>/compile_commands.json ./compile_commands.json
```

## 环境验收

环境完成必须满足：

- `check-env.sh` 的 required 项全部通过。
- Node 报告的三项版本与课程基线一致。
- 工具链冒烟程序能构建、运行、被 gdb 断住并被 strace 观察。
- libuv debug build 通过官方测试。
- Windows Git 不会把 shell 脚本改成 CRLF。

Windows 环境验收对应为：

- `check-windows.ps1` 的 required 项通过。
- Node/libuv/V8 版本与 Rocky 基线一致。
- 冒烟程序能产生 PE `.exe`、被调试器断住，并能用 `dumpbin` 查看符号/依赖。
- libuv 的 Windows debug build 通过官方 CTest。
