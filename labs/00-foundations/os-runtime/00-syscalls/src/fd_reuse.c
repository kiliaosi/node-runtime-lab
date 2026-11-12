// OS00 exercise: observe descriptor reuse and integer copies.
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  // The task runs this program from build/f03-os00.
  // TODO: Open "reuse-first.txt" using O_WRONLY | O_CREAT | O_TRUNC, 0644.

  int first_fd = open("reuse-first.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  // TODO: Check open's result; save the valid first fd in an int named saved.

  if (first_fd == -1) {
    perror("open first");
    return -1;
  }

  // TODO: Close first, check the result, then set first = -1.

  int saved = first_fd;

  if (close(first_fd) == -1) {
    perror("close first");
    return -1;
  }

  first_fd = -1;

  // TODO: Open "reuse-second.txt" using the same flags and check the result.

  int second_fd = open("reuse-second.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  // TODO: Print first, saved, and second with %d.

  if (second_fd == -1) {
    perror("open second");
    return -1;
  }

  printf("first=%d, saved=%d, second=%d", first_fd, saved, second_fd);

  // TODO: Close second exactly once and check the result.

  if (close(second_fd) == -1) {
    perror("close second");
    return -1;
  }

  return 0;
}
