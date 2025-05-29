
# 프로세스 제어 요약 

## 9.1 프로세스 생성

### fork() 시스템 호출

* 부모 프로세스를 복제하여 자식 프로세스를 생성
* 리턴값:

  * 자식 프로세스: 0
  * 부모 프로세스: 자식 PID

### 예제: fork1.c

```c
pid = fork();
printf("[%d] 리턴값 %d\n", getpid(), pid);
```

### 예제: fork2.c

```c
if (pid == 0) {
  printf("[Child]\n");
} else {
  printf("[Parent]\n");
}
```

### 예제: fork3.c (두 개의 자식 생성)

```c
pid1 = fork();
...
pid2 = fork();
```

## 9.1.1 자식 프로세스 기다리기

### wait()

* 자식 중 하나가 끝날 때까지 기다림

```c
child = wait(&status);
```

### waitpid()

* 특정 자식 프로세스를 기다림

```c
waitpid(pid1, &status, 0);
```

## 9.2 프로그램 실행

### exec() 계열 함수

* 현재 프로세스를 새로운 프로그램으로 대체
* 성공 시 리턴 없음

```c
execl("/bin/echo", "echo", "hello", NULL);
```

### 예제: execute3.c (명령줄 인수로 실행)

```c
execvp(argv[1], &argv[1]);
```

### system() 함수

* /bin/sh -c 명령어 형태로 실행

```c
system("date > file");
```

## 9.3 입출력 재지정

### dup2() 사용

* 표준 출력 재지정

```c
fd = open("file", ...);
dup2(fd, 1);
```

### 예제: redirect1.c

```c
printf("stdout\n");
fprintf(stderr, "stderr\n");
```

### 예제: redirect2.c

* 자식 프로세스의 출력을 파일로 재지정하고 exec 실행

## 9.4 프로세스 그룹

### getpgrp()

* 현재 프로세스의 그룹 ID를 가져옴

### setpgid()

* 새로운 그룹 생성 또는 기존 그룹에 참여

```c
setpgid(0, 0);
```

### waitpid() 그룹 옵션

* pid < -1 : 해당 그룹 ID의 자식 프로세스 종료 대기

## 9.5 시스템 부팅

* 커널이 만든 프로세스: swapper (스케줄러)
* init 프로세스 → /etc/inittab 기반 초기화
* getty → 로그인 프롬프트
* login → 사용자 인증
* shell → 명령어 대기 상태 진입

## 핵심 요약

* **fork()**: 부모 복제 → 자식 생성
* **exec()**: 기존 프로세스를 새로운 프로그램으로 대체
* **wait() / waitpid()**: 자식 종료 대기
* **dup2()**: 입출력 재지정
* **system()**: 명령어 실행
* **프로세스 그룹**: 시그널 전달 등에 사용
* **부팅 과정**: fork/exec 체계를 사용
