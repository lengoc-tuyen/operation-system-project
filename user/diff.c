#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXLINE 512

// Read one line (without the newline) into buf.
// Return its length, or -1 at end of file.
static int
readline(int fd, char *buf, int max)
{
  int i = 0;
  char c;

  while(i < max - 1){
    if(read(fd, &c, 1) != 1){
      if(i == 0)
        return -1;
      break;
    }
    if(c == '\n')
      break;
    buf[i++] = c;
  }
  buf[i] = 0;
  return i;
}

int
main(int argc, char *argv[])
{
  char line1[MAXLINE], line2[MAXLINE];
  char *file[2];
  int nfile = 0, quiet = 0;
  int fd1, fd2, n1, n2;

  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-q") == 0)
      quiet = 1;
    else if(nfile < 2)
      file[nfile++] = argv[i];
    else
      nfile++;
  }
  if(nfile != 2){
    fprintf(2, "usage: diff file1 file2 [-q]\n");
    exit(1);
  }

  if((fd1 = open(file[0], O_RDONLY)) < 0){
    fprintf(2, "diff: cannot open %s\n", file[0]);
    exit(1);
  }
  if((fd2 = open(file[1], O_RDONLY)) < 0){
    fprintf(2, "diff: cannot open %s\n", file[1]);
    close(fd1);
    exit(1);
  }

  for(int line = 1; ; line++){
    n1 = readline(fd1, line1, MAXLINE);
    n2 = readline(fd2, line2, MAXLINE);
    if(n1 < 0 && n2 < 0)
      break;
    if(n1 >= 0 && n2 >= 0 && strcmp(line1, line2) == 0)
      continue;

    if(quiet){
      printf("diff: files differ\n");
      break;
    }
    printf("%s:%d: < %s\n", file[0], line, n1 < 0 ? "EOF" : line1);
    printf("%s:%d: > %s\n", file[1], line, n2 < 0 ? "EOF" : line2);
  }

  close(fd1);
  close(fd2);
  exit(0);
}
