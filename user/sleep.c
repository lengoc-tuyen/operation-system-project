
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // Kiểm tra số lượng đối số, argc = 2 bao gồm tên chương trình sleep và đúng 1 đối số truyền vào
    if (argc != 2)
    {
        fprintf(2, "Usage: sleep ticks\n");
        exit(1);
    }

    int ticks = atoi(argv[1]);
    // Kiểm tra số tick hợp lệ
    if (ticks < 0)
    {
        fprintf(2, "sleep: invalid number of ticks\n");
        exit(1);
    }

    // Gọi system call sleep để tạm dừng tiến trình trong số tick đã cho
    sleep(ticks);
    printf("(nothing happened for a little while)\n");
    exit(0);
}
