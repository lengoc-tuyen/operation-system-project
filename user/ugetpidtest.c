#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int a = getpid();
  int b = ugetpid();
  printf("getpid=%d ugetpid=%d %s\n", a, b, a == b ? "OK" : "FAIL");
  exit(0);
}