#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    printf("[부모] fork() 호출 전\n");

    pid = fork();  // 여기서 복사가 일어남

    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }

    if (pid == 0) {
        // 자식 프로세스
        printf("[자식] 나는 자식 프로세스입니다. PID = %d, 부모 PID = %d\n", getpid(), getppid());
        printf("[자식] 여기서 exec()를 호출하면 다른 프로그램으로 대체됩니다.\n");
        exit(0);
    } else {
        // 부모 프로세스
        printf("[부모] 나는 부모 프로세스입니다. 자식 PID = %d, 내 PID = %d\n", pid, getpid());
        wait(NULL);  // 자식이 종료될 때까지 기다림
        printf("[부모] 자식이 종료됨. 이제 부모도 종료.\n");
    }

    return 0;
}
