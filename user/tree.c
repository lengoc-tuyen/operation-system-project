#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Hàm đệ quy duyệt cây thư mục
void treeTraversal(char *path, int level, int maxDepth, int onlyDir)
{
    int fd;
    struct stat st;
    char buffer[512];
    char *p;

    // Mở đường dẫn hiện tại
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    // Lấy thông tin của file/thư mục
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // Nếu là file thì chỉ cần in ra và kết thúc
    if (st.type != T_DIR)
    {
        if (!onlyDir)
        {
            printf("%s\n", path);
        }
        close(fd);
        return;
    }

    // Nếu đã đạt giới hạn độ sâu thì không duyệt sâu hơn
    if (maxDepth >= 0 && level >= maxDepth)
    {
        close(fd);
        return;
    }

    // Chuẩn bị buffer để tạo đường dẫn của phần tử con
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buffer))
    {
        fprintf(2, "tree: path too long\n");
        close(fd);
        return;
    }
    strcpy(buffer, path);
    p = buffer + strlen(buffer);
    *p++ = '/';

    // Đọc từng entry trong thư mục
    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        // Bỏ qua entry không hợp lệ
        if (de.inum == 0)
            continue;
        // Bỏ qua "." và ".."
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        // Tạo đường dẫn đầy đủ cho entry hiện tại
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        // Lấy thông tin của entry (file descriptor của entry con)
        int fdEntry = open(buffer, O_RDONLY);
        if (fdEntry < 0)
            continue;
        if (fstat(fdEntry, &st) < 0)
        {
            close(fdEntry);
            continue;
        }
        close(fdEntry);

        // Nếu dùng -d thì bỏ qua file
        if (onlyDir && st.type != T_DIR)
            continue;

        // In theo cấp độ
        for (int i = 0; i < level; i++)
            printf("|   ");
        printf("|-- %s\n", de.name);

        // Nếu là thư mục thì tiếp tục duyệt đệ quy
        if (st.type == T_DIR)
        {
            treeTraversal(buffer, level + 1, maxDepth, onlyDir);
        }
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    char *path = ".";
    int maxDepth = -1;
    int onlyDir = 0;

    // Phân tích các tham số dòng lệnh
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0) // Xử lý tùy chọn -d
        {
            onlyDir = 1;
        }
        else if (strcmp(argv[i], "-L") == 0) // Xử lý tùy chọn -L depth
        {
            if (i + 1 >= argc)
            {
                fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
                exit(1);
            }
            maxDepth = atoi(argv[++i]);
        }
        else if (argv[i][0] != '-') // Tham số còn lại được xem là path
        {
            path = argv[i];
        }
        else
        {
            fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
            exit(1);
        }
    }

    // In đường dẫn gốc
    printf("%s\n", path);
    // Bắt đầu duyệt cây từ path
    treeTraversal(path, 0, maxDepth, onlyDir);

    exit(0);
}