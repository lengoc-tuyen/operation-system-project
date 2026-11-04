#include "kernel/types.h"
#include "user.h"

// __attribute__ để nói hàm này exit, tức là ko quay lại nữa dừng tiến trình luôn
void sieve(int read_pipe) __attribute__((noreturn));

void sieve(int read_pipe)
{
    int num;
    int prime;
    int n = read(read_pipe, &num, sizeof(int));

    if (n == sizeof(int)) {
        prime = num;
        printf("prime %d\n", prime);
    }
    else {
        close(read_pipe);
        exit(n == 0 ? 0 : 1);
    }

    int p_new[2];
    pipe(p_new);
    int pid = fork();
    if (pid < 0) {
        close(read_pipe);
        close(p_new[0]);
        close(p_new[1]);
        exit(1);
    }
    else if (pid > 0) {
        close(p_new[0]);
        while (read(read_pipe, &num, sizeof(int)) == sizeof(int)) {
            if (num % prime != 0) write(p_new[1], &num, sizeof(int));
        }
        close(read_pipe);
        close(p_new[1]);
        wait(0);
    }
    else {
        close(read_pipe);
        close(p_new[1]);
        sieve(p_new[0]);
    }
  exit(0);
}

int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid < 0) {
        close(p[0]);
        close(p[1]);
        exit(1);
    }
    else if (pid == 0) {
        close(p[1]);
        sieve(p[0]);
    }
    else {
        close(p[0]);
        for (int i = 2; i <= 280; i++) {  
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
        wait(0);
        
    }
    exit(0);
}