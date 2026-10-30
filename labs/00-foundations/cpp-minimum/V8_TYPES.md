# F02：V8 Maybe 与局部句柄阅读练习

日期：2026-10-30。源码：[src/v8_maybe.cc](src/v8_maybe.cc)。

本练习使用 Node.js 附带的真实 V8 头文件，仅操作 Maybe<int> 和空句柄。
无需初始化 V8 或链接引擎库；没有创建、执行或回收真实 JavaScript 对象。

## 构建与运行

```bash
cd /root/workspace/node-runtime-lab &&
source scripts/activate-lab.sh &&
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug &&
cmake --build build/f02-cpp-minimum --target f02-v8-maybe &&
./build/f02-cpp-minimum/f02-v8-maybe
```

CMake 从 node 可执行文件附近查找 include/node/v8-maybe.h。
也可在配置时显式传入 -DF02_V8_INCLUDE_DIR=/path/to/node/include/node。
没有找到头文件时跳过本目标，其余基础实验仍可构建。
实际课堂头文件来自 /opt/node-v24.20.0-linux-x64/include/node。

VS Code：F1 → Tasks: Run Task → F02: build and run V8 Maybe。
构建会生成 compile_commands.json，可将其提供给编辑器智能提示。

## 已验证行为

- Just<int>(42) 有值，IsJust() 为 1，FromMaybe(-1) 为 42。
- Nothing<int>() 没有值，IsJust() 为 0，FromMaybe(-1) 为 -1。
- FromMaybe 的备用值不会填入原来的空对象。
- value.To(&out) 成功写出 42；第二次成功调用会再次覆盖 out。
- out 重设为 -99 后，empty.To(&out) 返回 false 并保持 -99。
- test_maybe_local 创建默认空 Local<String>/MaybeLocal<String>，ToLocal 返回 false；当前学生代码只打印这个返回值 0。

实际输出保留学生日志的格式：

```text
maybe test
1
42
0
-1
是否取值成功1
值:42
修改值142
empty to: 0 -99
0
```

“修改值142”是成功状态 1 与整数 42 连续输出。
源码中 empty.IsJust() 上方仍有“打印1”的旧注释，以实际输出 0 为准。

## 阅读要点与证据边界

Local<T> 和 MaybeLocal<T> 都可以为空，两者对象当前的状态都是确定的。
MaybeLocal 在接口上表达调用可能没有有效结果，使用 ToLocal 检查并取出。
Local 可以直接包装成 MaybeLocal；反向使用 ToLocal，成功后才访问对象。

Maybe<T>::To 失败时保留输出变量；MaybeLocal<T>::ToLocal 按接口约定会把输出句柄设空。
本次输出句柄起初就为空，没有通过运行验证“旧非空句柄被清空”的情况。

HandleScope 管理局部句柄的有效期；作用域结束不等于 JavaScript 对象立即销毁。
对象回收由 V8 GC 根据引用情况决定。保存 Local 的副本不会延长所属 HandleScope 的生命周期。
有效的强 Global/Persistent 句柄能保持对象存活；Reset 或 Global 析构解除对应引用。
弱句柄本身不阻止回收。

HandleScope、字符串创建和持久句柄仅做接口阅读，未在本练习中初始化引擎或运行这些行为。
相关接口：[Maybe](https://v8.github.io/api/head/classv8_1_1Maybe.html)、
[Local](https://v8.github.io/api/head/classv8_1_1Local.html)、
[MaybeLocal](https://v8.github.io/api/head/classv8_1_1MaybeLocal.html)。
