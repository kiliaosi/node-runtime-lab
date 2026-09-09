# Node Runtime Lab

这是为我定制的 Node.js 运行时进阶课程仓库。它不从 Express、数据库或管理系统开始，而是沿着真实依赖关系自底向上学习：

```text
Linux / C / C++
       ↓
libuv API → libuv 源码
       ↓
V8 Embedder API → V8 源码
       ↓
Node.js C++/JS 胶水层 → 核心模块 → 诊断与贡献
```

我已经有 Node.js 后端与 JavaScript 经验，本课程的目的不是重学语法，而是把“会使用 Node”推进到以下能力：

- 能用 C/C++ 写出小型 libuv 程序和 V8 embedder。
- 能从 JavaScript API 一路追踪到 Node C++ binding、V8 或 libuv，再到系统调用。
- 能解释事件循环、线程池、微任务、GC、Worker 和异步资源的真实边界。
- 能编译、调试、插桩和修改 Node.js，并用测试验证改动。
- 遇到性能、内存、句柄泄漏和异步时序问题时，能拿到证据而不是猜测。

## 版本基线

课程固定使用一组彼此对应的版本，避免拿最新版文档解释旧源码：

| 组件 | 课程版本 | 用途 |
| --- | --- | --- |
| Node.js | `v24.20.0` LTS | 主运行时与 Node 源码 |
| libuv | `v1.52.1` | 独立 API 实验与源码阅读 |
| V8 | `13.6.233.17-node.53` | 以 Node `v24.20.0/deps/v8` 为准 |
| Linux | Rocky Linux 9.8 x86_64 | 公司环境；Linux 编译、调试、系统调用观察 |
| Windows | Windows 11 x64（原生） | 家庭环境；日常学习、公共实验、Windows 后端实验 |
| C/C++ | C11、C++20 | libuv 与 V8/Node 源码所需最小集合 |

只有在专门研究版本差异时才升级。详细规则见 [docs/ENVIRONMENT.md](docs/ENVIRONMENT.md)。

## 仓库导航

- [完整课程大纲](docs/CURRICULUM.md)：阶段、知识点、实验、源码入口和验收标准。
- [环境搭建](docs/ENVIRONMENT.md)：原生 Windows 11 与 Rocky 9 两套工作环境。
- [平台运行矩阵](docs/PLATFORM_MATRIX.md)：每类实验在哪台机器执行，以及对应工具。
- [源码阅读方法](docs/SOURCE_READING.md)：从 API、实验和调用链进入源码。
- [官方资料索引](docs/RESOURCES.md)：只收录官方文档、官方仓库和原始设计资料。
- [进度表](PROGRESS.md)：每节课完成后留下证据。
- [第 0 阶段](labs/00-foundations/README.md)：开始前的 C/C++ 与 Linux 最小基础。

## 两台机器如何接力

GitHub 仓库是代码、笔记和进度的唯一同步源；上游源码、编译产物和 trace 只保留在各自机器上，不提交。每次换机器先 `git pull`，学习结束提交代码与笔记再 `git push`。

课程不要求两台机器重复完成所有实验：标记为 `COMMON` 的实验任选一台完成，`LINUX` 在公司 Rocky 运行，`WINDOWS` 在家里原生 Windows 运行，`COMPARE` 在两个平台各留一份结果。

## Rocky 9 第一次启动

在 Linux 开发机中克隆本仓库后执行：

```bash
cd ~/workspace/node-runtime-lab
bash scripts/bootstrap-rocky9.sh
source scripts/activate-lab.sh
bash scripts/check-env.sh
bash scripts/fetch-sources.sh libuv
cmake -S labs/00-foundations/toolchain-smoke -B build/toolchain-smoke
cmake --build build/toolchain-smoke
./build/toolchain-smoke/runtime-smoke
```

## Windows 11 第一次启动

Windows 不依赖 WSL。先安装 Git、CMake、Node `v24.20.0`，以及带“使用 C++ 的桌面开发”、ClangCL 和 Windows SDK 的 Visual Studio 2022/Build Tools。然后在 PowerShell 中执行：

```powershell
cd C:\workspace\node-runtime-lab
. .\scripts\activate-windows.ps1
.\scripts\check-windows.ps1
.\scripts\fetch-sources.ps1 libuv
cmake -S .\labs\00-foundations\toolchain-smoke -B .\build\toolchain-smoke-win -A x64
cmake --build .\build\toolchain-smoke-win --config Debug
.\build\toolchain-smoke-win\Debug\runtime-smoke.exe
```

完整安装项、Windows 调试工具和路径策略见 [docs/ENVIRONMENT.md](docs/ENVIRONMENT.md)。

`fetch-sources.sh` 会把大体积上游源码放在 `/data/node-runtime-lab/sources`，不会提交进课程仓库。进入 Node 阶段时再运行 `bash scripts/fetch-sources.sh node`。独立 V8 checkout 很大，等进入 V8 阶段再按环境文档获取；前期先学习 V8 公共 API，之后以 Node 内嵌的同版 V8 核对实现。

## 每节课的固定闭环

1. 用一张图或一段话说明对象、生命周期和线程归属。
2. 只学完成本节实验所需的 API。
3. 写一个正常实验和一个故意失败的实验。
4. 用 `gdb`、`strace`、日志或性能工具拿到运行证据。
5. 从公开 API 追到一条最短源码调用链。
6. 不看资料回答检查题，并把结论写进 `notes/`。

完成实验不等于学会；能够预测输出、解释原因、定位源码并改变一个条件验证预测，才算通过。

## 学习节奏

基准节奏为每周 8–12 小时、约 28 周。已有知识可以通过阶段测验后跳过，但不按“看完多少页”计进度，只按产出验收。V8 是一个长期方向，本课程的目标是建立可靠入口与主干认知，不承诺一次通读整个引擎。

## 当前起点

从 `F00` 环境与工具链开始，然后进入 `L01`。在真正学习 Node 事件循环之前，先自己使用 libuv 造出一个事件循环程序；在阅读 Node binding 之前，先自己用 V8 API 把 C++ 函数暴露给 JavaScript。
