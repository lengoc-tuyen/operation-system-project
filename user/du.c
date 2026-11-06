#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Return the total size in bytes of the regular files under path.
// A directory's own size is not counted. Directories are printed when
// print is set; files are printed only with -a, or when path itself is
// a file named on the command line (top).
static int
du(char *path, int all, int print, int top)
{
  char buf[512], *p;
  struct dirent de;
  struct stat st;
  int fd, total = 0;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "du: cannot open %s\n", path);
    return 0;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "du: cannot stat %s\n", path);
    close(fd);
    return 0;
  }

  if(st.type != T_DIR){
    close(fd);
    if(print && (all || top))
      printf("%d\t%s\n", (int)st.size, path);
    return st.size;
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
    fprintf(2, "du: path too long\n");
    close(fd);
    return 0;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    total += du(buf, all, print, 0);
  }
  close(fd);

  if(print)
    printf("%d\t%s\n", total, path);
  return total;
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int all = 0, summary = 0;

  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-a") == 0)
      all = 1;
    else if(strcmp(argv[i], "-s") == 0)
      summary = 1;
    else if(argv[i][0] != '-')
      path = argv[i];
    else {
      fprintf(2, "usage: du [path] [-a] [-s]\n");
      exit(1);
    }
  }

  if(summary)
    printf("%d\t%s\n", du(path, 0, 0, 1), path);
  else
    du(path, all, 1, 1);
  exit(0);
}
