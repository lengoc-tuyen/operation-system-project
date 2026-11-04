#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int du(char *path, int all, int print, int top)
{
  int fd = open(path, 0);
  if (fd < 0)
  { // mở file
    fprintf(2, "du: cannot open %s\n", path);
    return 0;
  }

  struct stat st;
  if (fstat(fd, &st) < 0)
  { // ghi thông tin file vào st
    fprintf(2, "du: cannot stat %s\n", path);
    close(fd);
    return 0;
  }

  int total = 0;
  if (st.type != T_DIR)
  { // ktra file thường
    total = (int)st.size;
    if (print && (all || top))
      printf("%d\t%s\n", total, path);
    close(fd);
    return total;
  }

  struct dirent de;
  char *buf = malloc(512);
  char *p;
  strcpy(buf, path); // buf = "path"
  p = buf + strlen(buf);
  *p++ = '/'; // buf = "path/"

  while (read(fd, &de, sizeof(de)) == sizeof(de))
  {
    if (de.inum == 0)
      continue; // ô trống
    if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
      continue;                      // bỏ qua . và ..
    memmove(p, de.name, DIRSIZ);     // buf = "path/tên"
    p[DIRSIZ] = 0;                   // tên đủ 14 ký tự thì không có '\0'
    total += du(buf, all, print, 0); // đệ quy rồi cộng dồn
  }
  close(fd);
  free(buf);

  if (print)
    printf("%d\t%s\n", total, path); // in sau khi cộng xong các con
  return total;
}

int main(int argc, char *argv[])
{
  char *path = ".";
  int all = 0, sum = 0;

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-a"))
      all = 1;
    else if (!strcmp(argv[i], "-s"))
      sum = 1;
    else
      path = argv[i];
  }

  if (sum)
    printf("%d\t%s\n", du(path, 0, 0, 1), path); // tắt in, chỉ lấy tổng
  else
    du(path, all, 1, 1);
  exit(0);
}