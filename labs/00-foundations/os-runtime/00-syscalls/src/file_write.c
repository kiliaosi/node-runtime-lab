// OS00 follow-up: implement the open -> write -> close lifecycle.
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  // The provided task runs this program from build/f03-os00.

  int fd = open("owned-output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd == -1) {
    perror("opern file");
    return -1;
  }

  char text[] = "hello file\n";
  ssize_t result = write(fd, text, sizeof(text) - 1);

  if (result == -1) {
    perror("write");
    if (close(fd) == -1) {
      perror("close");
    }

    return -1;
  }

  if (result != sizeof(text) - 1) {
    if (close(fd) == -1) {
      perror("close");
    }
    return 1;
  }

  if (close(fd) == -1) {
    perror("close");
    return 1;
  }
  return 0;
}
