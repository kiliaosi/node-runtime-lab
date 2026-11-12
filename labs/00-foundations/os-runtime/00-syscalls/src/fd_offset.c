// OS00 练习：比较 dup 共享偏移与两次 open 的独立偏移。
// 运行目录：build/f03-os00。
// 两组预期内容分别为 ABCX（4 字节）和 XBC（3 字节）。

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int test_dup_offset(void);
int test_open_offset(void);

int main(void) {
  // TODO: 调用 test_dup_offset，检查返回值。
  int result1 = test_dup_offset();

  printf("\nresult1 = %d", result1);

  // TODO: 第一组成功后调用 test_open_offset，检查返回值。
  int result2 = test_open_offset();

  printf("\nresult2 = %d", result2);

  // TODO: 两组都成功后返回 0，失败则返回非零。

  if (result1 == 0 && result2 == 0) return 0;
  return 1;
}

int test_dup_offset(void) {
  // TODO: 打开 dup-offset.txt，保存为 a。
  //       标志 O_WRONLY | O_CREAT | O_TRUNC，权限 0644。

  ssize_t f01 = open("dup-offset.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  // TODO: b = dup(a)，在写入前取得两个有效描述符。

  ssize_t f02 = dup(f01);

  // TODO: 通过 a 写入 "ABC"（3 字节），再通过 b 写入 "X"（1 字节）。

  ssize_t result = write(f01, "ABC", 3);
  // TODO: 用 ssize_t 保存每次写入结果，分别检查失败和短写。

  if (result != 3) {
    perror("write");
    return -1;
  }

  ssize_t result2 = write(f02, "X", 1);
  if (result2 != 1) {
    perror("write");
    return -1;
  }

  // TODO: 打印 a、b（%d）和两次写入结果（%zd），输出末尾加换行。

  printf("a=%ld, write result=%zd; b=%ld, write result=%zd", f01, result, f02, result2);

  // TODO: 分别关闭 a、b，检查结果。若一个关闭失败，仍需关闭另一个。

  int close1 = close(f01);

  int result_flag = 0;

  if (close1 == -1) {
    perror("close");
    result_flag = -1;
  }

  int close2 = close(f02);

  if (close2 == -1) {
    perror("close");
    result_flag = -1;
  }

  // TODO: 失败路径用 perror 或短写日志说明原因，清理已取得的描述符。
  // TODO: 完成后按实际执行结果返回 0 或非零。
  return result_flag;
}

int test_open_offset(void) {
  // TODO: 第一次打开 open-offset.txt，保存为 a。
  //       标志 O_WRONLY | O_CREAT | O_TRUNC，权限 0644。
  ssize_t f01 = open("open-offset.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  // TODO: 第二次打开同一路径，保存为 b，仅使用 O_WRONLY。

  ssize_t f02 = open("open-offset.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  //       两个 open 都成功后再开始写入。
  if (f01 == -1 || f02 == -1) {
    perror("open");
    return -1;
  }
  // TODO: 通过 a 写入 "ABC"（3 字节），再通过 b 写入 "X"（1 字节）。

  ssize_t result = write(f01, "ABC", 3);
  // TODO: 用 ssize_t 保存每次写入结果，分别检查失败和短写。

  if (result != 3) {
    perror("write");
    return -1;
  }

  ssize_t result2 = write(f02, "X", 1);
  if (result2 != 1) {
    perror("write");
    return -1;
  }

  // TODO: 打印 a、b（%d）和两次写入结果（%zd），输出末尾加换行。

  printf("a=%ld, write result=%zd; b=%ld, write result=%zd", f01, result, f02, result2);

  // TODO: 分别关闭 a、b，检查结果。若一个关闭失败，仍需关闭另一个。

  int close1 = close(f01);

  int result_flag = 0;

  if (close1 == -1) {
    perror("close");
    result_flag = -1;
  }

  int close2 = close(f02);

  if (close2 == -1) {
    perror("close");
    result_flag = -1;
  }

  // TODO: 失败路径用 perror 或短写日志说明原因，清理已取得的描述符。
  // TODO: 完成后按实际执行结果返回 0 或非零。
  return result_flag;
}
