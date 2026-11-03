#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/ptree.h"
#include "user/user.h"

static void
fail(char *message)
{
  printf("ptreetest: FAIL: %s\n", message);
  exit(1);
}

static int
find_pid(struct ptreeinfo *processes, int count, int pid)
{
  for(int i = 0; i < count; i++){
    if(processes[i].pid == pid)
      return i;
  }
  return -1;
}

int
main(int argc, char *argv[])
{
  struct ptreeinfo processes[NPROC];
  struct ptreeinfo limited[2];
  int parent_pid;
  int child_pid;
  int status;
  int count;

  if(ptree(0, NPROC) != -1)
    fail("accepted a null buffer");
  if(ptree(processes, 0) != -1)
    fail("accepted max equal to zero");
  if(ptree(processes, -1) != -1)
    fail("accepted a negative max");
  if(ptree((struct ptreeinfo *)0xffffffffffffffffULL, 1) != -1)
    fail("accepted an invalid buffer");

  memset(limited, 0, sizeof(limited));
  limited[1].pid = 0x12345678;
  if(ptree(limited, 1) != 1)
    fail("did not respect max equal to one");
  if(limited[1].pid != 0x12345678)
    fail("wrote beyond max entries");

  count = ptree(processes, NPROC);
  if(count <= 0 || count > NPROC)
    fail("returned an invalid process count");

  for(int i = 0; i < count; i++){
    if(processes[i].pid <= 0)
      fail("returned a non-positive pid");
    if(processes[i].state == 0)
      fail("returned an unused process");
    if(processes[i].name[sizeof(processes[i].name) - 1] != 0)
      fail("returned a name without a terminator");
    if(processes[i].ppid != 0 &&
       find_pid(processes, count, processes[i].ppid) < 0)
      fail("returned a process without its parent");
  }

  if(find_pid(processes, count, getpid()) < 0)
    fail("did not return the calling process");

  parent_pid = getpid();
  child_pid = fork();
  if(child_pid < 0)
    fail("fork failed");
  if(child_pid == 0){
    int child_index;

    count = ptree(processes, NPROC);
    child_index = find_pid(processes, count, getpid());
    if(child_index < 0 || processes[child_index].ppid != parent_pid)
      fail("returned an incorrect parent pid");
    exit(0);
  }
  if(wait(&status) != child_pid || status != 0)
    fail("child relationship test failed");

  printf("ptreetest: OK\n");
  exit(0);
}
