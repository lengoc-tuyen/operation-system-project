#include "kernel/types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int p1[2]; //cha --> p1 gửi vào p2 và wait sau đó nhận
    int p2[2]; //con --> p2 nhận sau đó gửi về p1 
    char c1 = 't'; //--> con
    char c2 = 'T'; // -->cha
    pipe(p1);
    pipe(p2);
    int pid = fork();
    if (pid < 0) {
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        exit(1);
    } else if (pid == 0) {
        close(p2[1]);
        close(p1[0]);
        read(p2[0], &c1, 1);
        printf("%d: received ping\n", getpid());
        close(p2[0]);
        write(p1[1], &c2, 1);
        close(p1[1]);
        exit(0);
    } else {
        close(p2[0]);
        close(p1[1]);
        write(p2[1], &c1, 1);
        wait(0);
        read(p1[0], &c2, 1);
        printf("%d: received pong\n", getpid());
        close(p1[0]);
        close(p2[1]);
    }
    exit(0);
}