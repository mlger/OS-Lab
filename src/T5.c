#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int m, n, k;
    m = fork();
    printf("lg      ");
    printf("pid:%d   ", getpid());
    printf("the return value:%d\n", m);

    n = fork();
    printf("lgg     ");
    printf("pid:%d   ", getpid());
    printf("the return value:%d\n", n);

    k = fork();
    printf("lggg    ");
    printf("pid:%d   ", getpid());
    printf("the return value:%d\n", k);
}