#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/ptree.h"
#include "user/user.h"

static int
find_pid(struct ptreeinfo *processes, int count, int pid)
{
  for(int i = 0; i < count; i++){
    if(processes[i].pid == pid)
      return i;
  }
  return -1;
}

static void
print_process(struct ptreeinfo *process, int depth)
{
  for(int i = 0; i < depth; i++)
    printf("  ");
  printf("%d %s state=%d mem=%ld\n", process->pid, process->name,
         process->state, process->memsize);
}

static void
print_subtree(struct ptreeinfo *processes, int count, int index, int depth,
              char *printed)
{
  if(index < 0 || index >= count || printed[index])
    return;

  printed[index] = 1;
  print_process(&processes[index], depth);

  for(int i = 0; i < count; i++){
    if(processes[i].ppid == processes[index].pid)
      print_subtree(processes, count, i, depth + 1, printed);
  }
}

int
main(int argc, char *argv[])
{
  struct ptreeinfo processes[NPROC];
  char printed[NPROC];
  int count;

  if(argc != 1){
    fprintf(2, "usage: pstree\n");
    exit(1);
  }

  count = ptree(processes, NPROC);
  if(count < 0){
    fprintf(2, "pstree: ptree system call failed\n");
    exit(1);
  }

  memset(printed, 0, sizeof(printed));

  // Print normal roots first: no parent, or a parent absent from the snapshot.
  for(int i = 0; i < count; i++){
    if(processes[i].ppid == 0 ||
       find_pid(processes, count, processes[i].ppid) < 0)
      print_subtree(processes, count, i, 0, printed);
  }

  // A malformed or cyclic relationship must not hide a process.
  for(int i = 0; i < count; i++){
    if(!printed[i])
      print_subtree(processes, count, i, 0, printed);
  }

  exit(0);
}
