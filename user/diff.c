#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXL 512

// Đọc 1 dòng vào buf (bỏ '\n').
// Trả về độ dài dòng (>= 0), hoặc -1 nếu đã hết file và không đọc được gì.
int readline(int fd, char *buf, int max)
{
  int i = 0;
  char c;
  while (i < max - 1)
  {

    int r = read(fd, &c, 1); // đọc từng kí tự

    if (r <= 0)
    { // hết file (hoặc lỗi)
      if (i == 0)
        return -1; // chưa đọc được ký tự nào: hết dòng để đọc
      break;       // dòng cuối không có '\n'
    }

    if (c == '\n')
    { // gặp xuống dòng: kết thúc dòng
      buf[i] = 0;
      return i;
    }
    buf[i++] = c;
  }

  buf[i] = 0;
  return i;
}

int main(int argc, char *argv[])
{
  char *f[2];
  int nf = 0;    // số lượng file
  int quiet = 0; // xem có -q không

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-q"))
      quiet = 1;
    else if (nf < 2)
      f[nf++] = argv[i];
  }
  if (nf != 2)
  {
    fprintf(2, "usage: diff file1 file2 [-q]\n");
    exit(1);
  }

  int fd1 = open(f[0], 0); // đọc chỉ số trong bảng file descriptor
  int fd2 = open(f[1], 0);

  if (fd1 < 0 || fd2 < 0)
  {
    fprintf(2, "diff: cannot open %s\n", fd1 < 0 ? f[0] : f[1]);
    exit(1);
  }

  char b1[MAXL], b2[MAXL];
  for (int line = 1;; line++)
  {
    int r1 = readline(fd1, b1, MAXL); // r1 = độ dài dòng, b1 = nội dung dòng của file 1
    int r2 = readline(fd2, b2, MAXL); // r2 = độ dài dòng, b2 = nội dung dòng của file 2

    if (r1 < 0 && r2 < 0)
      break; // cả hai cùng hết

    if (r1 >= 0 && r2 >= 0 && strcmp(b1, b2) == 0)
      continue; // hai dòng giống nhau

    // đến đây là có khác biệt
    if (quiet)
    {
      printf("diff: files differ\n"); // nếu có -q thì chỉ in ra thông báo và thoát
      exit(0);
    }
    printf("%s:%d: < %s\n", f[0], line, r1 < 0 ? "EOF" : b1); // in ra dòng khác nhau của file 1
    printf("%s:%d: > %s\n", f[1], line, r2 < 0 ? "EOF" : b2); // in ra dòng khác nhau của file 2
  }
  close(fd1);
  close(fd2);
  exit(0);
}