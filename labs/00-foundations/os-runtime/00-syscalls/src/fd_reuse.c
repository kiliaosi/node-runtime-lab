// OS00 exercise: observe descriptor reuse and integer copies.
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  // The task runs this program from build/f03-os00.
  // TODO: Open "reuse-first.txt" using O_WRONLY | O_CREAT | O_TRUNC, 0644.
  // TODO: Check open's result; save the valid first fd in an int named saved.
  // TODO: Close first, check the result, then set first = -1.
  // TODO: Open "reuse-second.txt" using the same flags and check the result.
  // TODO: Print first, saved, and second with %d.
  // TODO: Close second exactly once and check the result.
  // TODO: Return 0 only if all required operations succeeded.
  fputs("Exercise pending: observe fd reuse.\n", stderr);
  return 1;
}
