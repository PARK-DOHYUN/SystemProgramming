#include <sys/types.h>
#include <sys/wait.h>     // waitpid 함수 사용
#include <unistd.h>       // fork, getpid, sleep 함수 사용
#include <stdio.h>        // printf 함수 사용
#include <stdlib.h>       // exit 함수 사용

int main() {
    int pid1, pid2, child, status;

    printf("[%d] 부모 프로세스 시작 \n", getpid());

    pid1 = fork();
    if (pid1 == 0) {
        // 자식 프로세스 1
        printf("[%d] 자식 프로세스[1] 시작 \n", getpid());
        sleep(1);
        printf("[%d] 자식 프로세스[1] 종료 \n", getpid());
        exit(1); // 종료 코드 1 반환
    }

    pid2 = fork();
    if (pid2 == 0) {
        // 자식 프로세스 2
        printf("[%d] 자식 프로세스 #2 시작 \n", getpid());
        sleep(2);
        printf("[%d] 자식 프로세스 #2 종료 \n", getpid());
        exit(2); // 종료 코드 2 반환
    }

    // 첫 번째 자식 프로세스가 종료될 때까지 대기
    child = waitpid(pid1, &status, 0);

    printf("[%d] 자식 프로세스 #1 %d 종료 \n", getpid(), child);
    printf("\t종료 코드 %d\n", status >> 8); // 하위 8비트를 시프트하여 종료 코드 출력
}
