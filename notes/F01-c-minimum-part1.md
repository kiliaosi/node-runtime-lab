# F01 C 最小集（一）：指针、结构体与内存生命周期

日期：2026-09-15  
状态：进行中

## 本次目标

为阅读 libuv 的 handle、request、回调和 `void *data` 补齐必要的 C 语言基础。本次全部实验代码均为手写，并使用严格编译参数验证。

## 1. 一级指针与二级指针

实验关系：

```c
int x = 42;
int *p = &x;
int **pp = &p;
```

需要能够直接读出：

```text
p    == &x
*pp  == p
pp   == &p
**pp == x
```

两种修改的含义不同：

```c
*pp = &y;    // 修改 p 保存的地址，让 p 改为指向 y
**pp = 1234; // 修改 p 当前指向的整数，此时修改的是 y
```

打印对象指针时使用 `%p`，并显式转换为 `void *`：

```c
printf("%p\n", (void *)p);
```

## 2. `struct`、`typedef` 与结构体指针

匿名结构体配合类型别名：

```c
typedef struct {
  int x;
  int y;
} Point;
```

这里定义了类型别名 `Point`，但没有定义结构体标签 `struct Point`。

结构体指针的两种成员访问方式等价：

```c
point_ptr->x == (*point_ptr).x
```

前置声明和自引用结构体：

```c
typedef struct Node Node;

struct Node {
  void *data;
  Node *next;
};
```

前置声明阶段的 `Node` 是不完整类型：编译器还不知道它的大小，但已经可以声明 `Node *`，因为指针大小是已知的。这种模式会在 libuv 的 `xxx_t` 类型中反复出现。

## 3. 结构体内存布局

结构体对齐的准确模型：

1. 成员按照声明顺序排列。
2. 上一个成员的结束位置决定当前偏移。
3. 放置下一个成员时，如果当前偏移不满足该成员的对齐要求，就在前一个成员之后插入内部填充。
4. 全部成员放完后，结构体总大小补齐为结构体最大对齐值的整数倍，这部分是尾部填充。

填充位于成员之间或结构体尾部，但不属于某个成员本身。不能把规则理解成“每个小成员都扩大到最大成员的大小”。

本次实测（Rocky Linux 9，x86-64）：

```text
sizeof(Point) = 8

User:
username 偏移 0，指针大小 8
password 偏移 8，指针大小 8
gender   偏移 16，int 大小 4
尾部填充 4
sizeof(User) = 24
```

成员顺序实验：

```text
LayoutA = char + int + char
成员偏移 = 0, 4, 8
sizeof(LayoutA) = 12

LayoutB = int + char + char
成员偏移 = 0, 4, 5
sizeof(LayoutB) = 8
```

这证明调整成员顺序可能减少内部填充和结构体总大小。

`sizeof` 和 `offsetof` 的结果类型都是 `size_t`，使用 `%zu` 输出。`offsetof(Type, member)` 是 `<stddef.h>` 提供的标准宏，现代编译器通常借助内建能力计算成员偏移。

## 4. 栈上单链表与 `void *`

手写了三个栈上节点，并形成：

```text
node1 -> node2 -> node3 -> NULL
```

使用 `Node *current` 和 `while (current != NULL)` 完成遍历。

`void *` 可以保存任意对象地址，但不携带所指对象的具体类型。按字符串使用前必须恢复类型：

```c
printf("%s\n", (char *)current->data);
```

栈上节点的生命周期属于 `link_list()` 的栈帧。函数返回后节点失效，不能把局部节点地址返回给外部长期使用。

## 5. 单个堆节点与所有权

单个堆对象的基本生命周期：

```text
malloc -> 检查 NULL -> 初始化 -> 使用 -> free -> 清空本地指针
```

推荐写法：

```c
Node *node = malloc(sizeof *node);
```

要点：

- C 中 `malloc` 返回的 `void *` 可以自动转换为对象指针，不必强制转换。
- `malloc` 返回的内存未初始化。
- `sizeof *node` 根据表达式类型计算大小，普通类型下不会真的解引用 `node`。
- `free(node)` 才执行释放；`node = NULL` 只是避免继续误用这个局部指针。
- 其他指向同一内存的别名不会自动变成 `NULL`，仍可能成为悬空指针。
- 当前 `data` 指向字符串字面量，节点只是借用地址，因此只释放节点，不释放 `data`。

初步所有权约定：创建函数返回堆对象后，调用者取得所有权并负责最终释放。

## 6. 编译验证

构建命令：

```bash
cmake -S labs/00-foundations/c-minimum \
  -B /data/node-runtime-lab/build/f01-c-minimum \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build /data/node-runtime-lab/build/f01-c-minimum \
  --target f01-pointers -j2

/data/node-runtime-lab/build/f01-c-minimum/f01-pointers
```

实验使用：

```text
-std=c11 -Wall -Wextra -Wpedantic -Werror
```

严格格式检查帮助发现了两类问题：

- `sizeof` 返回 `size_t`，不能用 `%d`，应使用 `%zu`。
- `%s` 要求字符指针，不能直接把 `void *` 作为对应参数传入。

## 当前进度

F01 约完成三分之一到四成。已经完成指针、结构体、布局、自引用结构体、栈上链表以及单个堆节点的基础实验，但尚未达到 F01 闸门。

## 下次起点

1. 清理练习代码中的重复函数声明、输出标签和缺失换行。
2. 实现 `Node *create_node(void *data)`，明确“创建者返回、调用者释放”的所有权契约。
3. 创建并安全释放动态链表，观察 use-after-free、double-free 和泄漏。
4. 继续数组退化、字符串、字节缓冲区。
5. 学习函数指针、回调和 `void *context`。
6. 完成头文件、翻译单元、宏和条件编译后，实现 F01 小型任务队列。
