# F02 C++ 最小集：对象生命周期与所有权

日期：2026-10-22
状态：本目录四组实验已通过严格构建与运行；F02 整体仍在进行中。

## 已完成的实验

源码：[src/main.cc](src/main.cc)。构造和析构日志由 Trace 提供。

| 函数 | 观察目标 |
| --- | --- |
| test_scope | 局部 Trace 在内层作用域结束时析构，destroy 在 C 之前 |
| test_unique | 局部 unique_ptr 管理动态 Trace；打印 B 后提前 return，仍然输出 destroy，不输出 C |
| test_move | 内层 q 通过 std::move(p) 接管所有权，p 为空；q 离开内层时销毁对象 |
| custom_trace / test_transfer | unique_ptr 按值形参通过移动接管所有权；调用结束时销毁对象，调用者 item 为空 |

main 依次调用 test_scope、test_unique、test_move、test_transfer。
最初 test_unique 的正常作用域退出版本也已验证；当前保存的是提前 return 版本。
使用 std::move 显式包含 <utility>，使用 unique_ptr/make_unique 包含 <memory>。

## Rocky Linux 构建与运行

```bash
cd /root/workspace/node-runtime-lab
source scripts/activate-lab.sh
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug
cmake --build build/f02-cpp-minimum --clean-first
timeout 5s ./build/f02-cpp-minimum/f02-lifetime
```

本实验独立配置，不依赖 libuv。使用 C++20，关闭语言扩展。
GCC/Clang 编译选项为 -Wall -Wextra -Wpedantic -Werror；MSVC 配置 /W4 /WX /permissive-。
本轮实际验证环境为 Rocky Linux / GNU C++ 14.2.1，未在 Windows 上验证此实验。

## 已核对的输出

```text
scope
A
create
B
destroy
C
unique
A
create
B
destroy
move
create
p empty: 1
q empty: 0
destroy
after inner scope
transfer
create
before call
inside custom_trace
item empty: 0
destroy
after call
caller item empty: 1
```

严格重新构建没有警告，运行退出 0；本轮未执行 sanitizer 或分配失败注入。

## 后续练习

亲手实现一个管理真实 C 资源的 RAII 类，继续处理资源生命周期、所有权和复制约束。
详细学习结论与纠正记录见 [F02 笔记（一）](../../../notes/F02-cpp-minimum-part1.md)。
课程范围见 [CURRICULUM.md](../../../docs/CURRICULUM.md)，总进度见 [PROGRESS.md](../../../PROGRESS.md)。
