#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
  exit(1);
}

// Print the entries of directory path, indented by level.
// maxdepth < 0 means no depth limit.
static void
tree(char *path, int level, int maxdepth, int onlydir)
{
  char buf[512], *p;
  struct dirent de;
  struct stat st;
  int fd;

  if(maxdepth >= 0 && level >= maxdepth)
    return;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    return;
  }
  if(st.type != T_DIR){
    close(fd);
    return;
  }
  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
    fprintf(2, "tree: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if(stat(buf, &st) < 0)
      continue;
    if(onlydir && st.type != T_DIR)
      continue;

    for(int i = 0; i < level; i++)
      printf("|   ");
    printf("|-- %s\n", p);

    if(st.type == T_DIR)
      tree(buf, level + 1, maxdepth, onlydir);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  struct stat st;
  int maxdepth = -1;
  int onlydir = 0;

  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      onlydir = 1;
    } else if(strcmp(argv[i], "-L") == 0){
      if(i + 1 >= argc)
        usage();
      maxdepth = atoi(argv[++i]);
    } else if(argv[i][0] != '-'){
      path = argv[i];
    } else {
      usage();
    }
  }

  if(stat(path, &st) < 0){
    fprintf(2, "tree: cannot open %s\n", path);
    exit(1);
  }
  printf("%s\n", path);
  tree(path, 0, maxdepth, onlydir);
  exit(0);
}
