#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  struct sysinfo info;

  // Gọi system call sysinfo và truyền vào địa chỉ của struct
  if (sysinfo(&info) < 0) {
    printf("sysinfotest: FAIL\n");
    exit(1);
  }

  // In ra dữ liệu thu thập được để verify thực tế
  printf("freemem: %ld bytes\n", info.freemem);
  printf("nproc: %ld\n", info.nproc);
  
  // In ra kết quả thành công
  printf("sysinfotest: OK\n");
  
  exit(0);
}