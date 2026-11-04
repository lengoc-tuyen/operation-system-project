#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[])
{
    int fd[2]; // Lưu 2 file descriptors
    char buffer[512];
    // Kiểm tra tham số
    if (argc != 3)
    {
        fprintf(2, "syntax request: cp src dst\n");
        exit(1);
    }

    // Mở file nguồn
    int src = open(argv[1], O_RDONLY);
    if (src < 0)
    {
        fprintf(2, "cp: cannot open %s\n", argv[1]);
        exit(1);
    }

    // Mở file đích để ghi / tạo file mới / xóa nội dung cũ trước khi ghi
    int dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC);
    if (dst < 0)
    {
        fprintf(2, "cp: cannot create %s\n", argv[2]);
        close(src);
        exit(1);
    }

    // Tạo pipe
    if (pipe(fd) < 0)
    {
        fprintf(2, "cp: pipe error\n");
        close(src);
        close(dst);
        exit(1);
    }

    // Tạo tiến trình con
    int pid = fork();
    if (pid < 0)
    {
        fprintf(2, "cp: fork error\n");
        close(src);
        close(dst);
        close(fd[0]);
        close(fd[1]);
        exit(1);
    }

    if (pid == 0)
    {
        // Tiến trình con: đọc dữ liệu từ pipe và ghi vào file đích
        close(fd[1]); // Tiến trình con không ghi pipe nên đóng đầu ghi
        close(src);   //  Tiến trình con không dùng file nguồn

        int n;
        // Đọc dữ liệu từ pipe cho đến khi hết dữ liệu
        while ((n = read(fd[0], buffer, sizeof(buffer))) > 0)
        {
            // Ghi dữ liệu đọc được vào file đích
            if (write(dst, buffer, n) != n)
            {
                fprintf(2, "cp: write error\n");
                close(fd[0]);
                close(dst);
                exit(1);
            }
        }

        // Kiểm tra lỗi khi đọc pipe
        if (n < 0)
        {
            fprintf(2, "cp: read pipe error\n");
        }
        close(fd[0]);
        close(dst);
        exit(0);
    }
    else
    {
        // Tiến trình cha: đọc file nguồn và ghi dữ liệu vào pipe
        close(fd[0]); // Tiến trình cha không đọc pipe nên đóng đầu đọc
        close(dst);   // Tiến trình cha không dùng file đích

        int n;
        // Đọc dữ liệu từ file nguồn
        while ((n = read(src, buffer, sizeof(buffer))) > 0)
        {
            // Ghi dữ liệu đọc được vào pipe
            if (write(fd[1], buffer, n) != n)
            {
                fprintf(2, "cp: write pipe error\n");
                close(src);
                close(fd[1]);
                wait(0);
                exit(1);
            }
        }

        // Kiểm tra lỗi khi đọc file nguồn
        if (n < 0)
        {
            fprintf(2, "cp: read error\n");
        }
        close(src);
        close(fd[1]);
        wait(0); // Chờ tiến trình con hoàn thành việc ghi file đích
        exit(0);
    }
}