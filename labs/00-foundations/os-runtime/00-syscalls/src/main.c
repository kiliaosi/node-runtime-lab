// OS00: compare an ordinary C call with a write system call.
#include <stdio.h>
#include <unistd.h>

int add(int a, int b);

int main(void) {
  int result = add(2, 3);

  if (result != 5) return -1;

  char text[] = "hello kernel\n";

  ssize_t write_result = write(STDOUT_FILENO, text, sizeof(text) - 1);

  if (write_result == -1) {
    perror("write");
    return -1;
  }

  if (write_result != 13) return 1;

  return 0;
}

int add(int a, int b) { return a + b; }
