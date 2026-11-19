#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_ptree(void)
{
  uint64 buffer;
  int max;

  argaddr(0, &buffer);
  argint(1, &max);
  return collect_ptree(buffer, max);
}

uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);
  myproc()->trace_mask = mask;
  return 0;
}

uint64
sys_sysinfo(void)
{
  uint64 address;
  struct sysinfo info;

  argaddr(0, &address);
  info.freemem = get_freemem();
  info.nproc = get_nproc();

  if(copyout(myproc()->pagetable, address, (char *)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}

// Kiểm tra các trang bộ nhớ user đã được truy cập (PTE_A) và trả về bitmask.
// Tham số user truyền vào: địa chỉ ảo bắt đầu, số lượng trang, địa chỉ buffer để ghi bitmask kết quả
uint64
sys_pgaccess(void)
{
  uint64 start_va;
  int npages;
  uint64 mask_va;

  // Lấy 3 tham số từ thanh ghi a0, a1, a2 của tiến trình user
  argaddr(0, &start_va);
  argint(1, &npages);
  argaddr(2, &mask_va);

  // Giới hạn số trang tối đa là 32 (tương ứng 32 bit của unsigned int)
  if(npages < 0 || npages > 32)
    return -1;

  // Căn xuống đầu trang để tránh lệch offset và kiểm tra tràn địa chỉ MAXVA
  uint64 va = PGROUNDDOWN(start_va);
  if(va >= MAXVA || (uint64)npages * PGSIZE > MAXVA - va)
    return -1;

  struct proc *p = myproc();
  unsigned int mask = 0; // bitmask tạm trong kernel lưu trạng thái truy cập

  for(int i = 0; i < npages; i++){
    // Tìm con trỏ PTE cấp 0 tương ứng với địa chỉ ảo của trang thứ i (không cấp phát mới)
    pte_t *pte = walk(p->pagetable, va + (uint64)i * PGSIZE, 0);

    // Nếu PTE hợp lệ và phần cứng đã bật bit PTE_A (trang đã được đọc/ghi)
    if(pte != 0 && (*pte & PTE_V) && (*pte & PTE_A)){
      mask |= (1U << i);  // đánh dấu bit thứ i trong bitmask
      *pte &= ~PTE_A;     // xóa bit PTE_A để lần gọi pgaccess sau chỉ đo truy cập mới
    }
  }

  // Chép bitmask kết quả từ kernel ra buffer của user
  if(copyout(p->pagetable, mask_va, (char *)&mask, sizeof(mask)) < 0)
    return -1;

  return 0;
}
