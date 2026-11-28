#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

// Hàm hỗ trợ in lỗi và thoát khi test thất bại
void
err(char *why)
{
  printf("pgaccesstest: FAIL: %s\n", why);
  exit(1);
}

// Kiểm tra chức năng phát hiện trang đã truy cập của syscall pgaccess()
void
pgaccess_test(void)
{
  unsigned int abits;

  printf("pgaccesstest: start\n");

  // Cấp phát 33 trang với PGSIZE = 4096 để đảm bảo mỗi trang không dính metadata của malloc
  char *raw = malloc(33 * PGSIZE);
  if(raw == 0)
    err("malloc failed");
  char *buf = (char *)PGROUNDUP((uint64)raw);

  // Lần 1: Gọi pgaccess để xóa sạch các bit PTE_A có sẵn (nếu có)
  if(pgaccess(buf, 32, &abits) < 0)
    err("pgaccess initial call failed");

  // Truy cập có chủ đích vào các trang số 1, 2 và 30 (đọc và ghi)
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;

  // Lần 2: Gọi pgaccess kiểm tra bitmask trả về có khớp với trang 1, 2, 30 không
  if(pgaccess(buf, 32, &abits) < 0)
    err("pgaccess second call failed");
  if(abits != ((1U << 1) | (1U << 2) | (1U << 30))){
    printf("expected 0x%x, got 0x%x\n", (1U << 1) | (1U << 2) | (1U << 30), abits);
    err("incorrect access bits set");
  }

  // Lần 3: Không truy cập trang nào nữa, bitmask trả về 0 (xác nhận PTE_A đã được xóa)
  if(pgaccess(buf, 32, &abits) < 0)
    err("pgaccess third call failed");
  if(abits != 0){
    printf("expected 0, got 0x%x\n", abits);
    err("access bits were not cleared");
  }

  // Kiểm tra các trường hợp tham số không hợp lệ
  if(pgaccess(buf, -1, &abits) != -1)
    err("negative npages should fail");
  if(pgaccess(buf, 33, &abits) != -1)
    err("npages > 32 should fail");
  if(pgaccess(buf, 4, (void *)0xdeadbeefdeadbeefULL) != -1)
    err("invalid mask pointer should fail");

  free(raw);
  printf("pgaccesstest: OK\n");
}

int
main(int argc, char *argv[])
{
  pgaccess_test();
  exit(0);
}
