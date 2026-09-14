# L01-1：第一个 libuv loop

目标：区分注册工作与驱动事件循环，观察 handle 的关闭如何影响 `uv_run()` 返回。

## 构建

Rocky Linux：

```bash
cd ~/workspace/node-runtime-lab
source scripts/activate-lab.sh
cmake -S labs -B "$NODE_RUNTIME_BUILD/course-labs" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$NODE_RUNTIME_BUILD/course-labs" --target l01-loop-basics
"$NODE_RUNTIME_BUILD/course-labs/01-libuv/01-loop-basics/l01-loop-basics"
```

Windows PowerShell：

```powershell
cd C:\workspace\node-runtime-lab
. .\scripts\activate-windows.ps1
cmake -S .\labs -B "$env:NODE_RUNTIME_BUILD\course-labs" -A x64
cmake --build "$env:NODE_RUNTIME_BUILD\course-labs" --config Debug --target l01-loop-basics
& "$env:NODE_RUNTIME_BUILD\course-labs\01-libuv\01-loop-basics\Debug\l01-loop-basics.exe"
```

## 运行前预测

1. 四行输出的顺序是什么？
2. `uv_timer_start()` 会不会等待 300 ms？
3. 真正发生等待的是哪一个调用？
4. 如果回调中不调用 `uv_close()`，`uv_loop_close()` 会返回什么状态？
