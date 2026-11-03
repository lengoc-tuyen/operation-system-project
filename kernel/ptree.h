#ifndef XV6_PTREE_H
#define XV6_PTREE_H

struct ptreeinfo {
  int pid;
  int ppid;
  int state;
  uint64 memsize;
  char name[16];
};

#endif
