#include "kernel/types.h"
#include "user/user.h"

#define LIMIT 280

// Each stage prints the first number it receives (a prime), forks the next
// stage, and forwards every number not divisible by that prime to it.
// The child continues the loop with the new pipe instead of recursing, so
// the user stack does not grow with the length of the pipeline.
static void
sieve(int input)
{
  int prime, num;
  int next[2];
  int pid;

  for(;;){
    if(read(input, &prime, sizeof(prime)) != sizeof(prime)){
      close(input);
      exit(0);
    }
    printf("prime %d\n", prime);

    if(pipe(next) < 0){
      fprintf(2, "primes: pipe failed\n");
      exit(1);
    }
    pid = fork();
    if(pid < 0){
      fprintf(2, "primes: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      close(input);
      close(next[1]);
      input = next[0];
      continue;
    }

    close(next[0]);
    while(read(input, &num, sizeof(num)) == sizeof(num)){
      if(num % prime != 0)
        write(next[1], &num, sizeof(num));
    }
    close(input);
    close(next[1]);
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int p[2];
  int pid;

  if(pipe(p) < 0){
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  pid = fork();
  if(pid < 0){
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    close(p[1]);
    sieve(p[0]);
  }

  close(p[0]);
  for(int i = 2; i <= LIMIT; i++)
    write(p[1], &i, sizeof(i));
  close(p[1]);
  wait(0);
  exit(0);
}
