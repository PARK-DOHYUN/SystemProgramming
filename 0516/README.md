# 파일 시스템 (File System)

##  링크 (Links)

### 하드 링크 (Hard Link)

* 기존에 다루었던 일반적인 링크 방식.

* **i-node를 직접 가리킴**.

* 같은 파일 시스템 내에서만 사용 가능.

* `link()` 함수 사용:

  ```c
  #include <unistd.h>
  int link(const char *existing, const char *new);
  ```

* 하드 링크 예시:

  ```c
  if (link(argv[1], argv[2]) == -1) {
      exit(1);
  }
  ```

### 심볼릭 링크 (Symbolic Link, Soft Link)

* **파일 경로명을 저장**하는 링크.

* 다른 파일 시스템의 파일도 링크할 수 있음.

* 간접적인 포인터 역할.

* `symlink()` 함수 사용:

  ```c
  #include <unistd.h>
  int symlink(const char *actualpath, const char *sympath);
  ```

* 심볼릭 링크 예시:

  ```c
  if (symlink(argv[1], argv[2]) == -1) {
      exit(1);
  }
  ```

* 예시 실행:

  ```sh
  $ slink /usr/bin/gcc cc
  $ ls -l cc
  lrwxrwxrwx 1 chang chang 7 4월 8 19:58 cc -> /usr/bin/gcc
  ```

### 심볼릭 링크 내용 확인

* `readlink()` 함수 사용:

  ```c
  #include <unistd.h>
  int readlink(const char *path, char *buf, size_t bufsize);
  ```

* 심볼릭 링크가 가리키는 **실제 경로를 문자열로 읽음**.

* 예제 코드:

  ```c
  char buffer[1024];
  int nread = readlink(argv[1], buffer, 1024);
  if (nread > 0) {
      write(1, buffer, nread);
      exit(0);
  } else {
      fprintf(stderr, "오류 : 해당 링크 없음\n");
      exit(1);
  }
  ```

---

## 핵심 요약

* **하드 링크**: 동일 파일 시스템, i-node 직접 연결.
* **심볼릭 링크**: 경로명 기반, 다른 파일 시스템도 링크 가능.
* 심볼릭 링크 내용은 `readlink()`로 확인 가능.
* 각각 `link()`, `symlink()` 함수로 생성.

---

# 파일 및 레코드 잠금 (File & Record Locking)

## 1 파일 잠금 (File Locking)

### 개요

* 여러 프로세스가 파일에 동시에 접근하면 데이터 무결성 문제가 발생할 수 있음.
* 이를 방지하기 위해 잠금(lock)을 사용함.

### flock() 함수

* 파일 전체에 대한 잠금.
* 잠금 종류:

  * `LOCK_SH`: 공유 잠금 (여러 프로세스 읽기 허용)
  * `LOCK_EX`: 배타 잠금 (단일 프로세스만 접근 가능)
  * `LOCK_UN`: 잠금 해제
  * `LOCK_NB`: 논블로킹 잠금 요청

```c
#include <sys/file.h>
int flock(int fd, int operation);
```

### 예제 코드

```c
fd = open(argv[1], O_WRONLY | O_CREAT, 0600);
if (flock(fd, LOCK_EX) != 0) {
    perror("flock error");
    exit(1);
}
...
flock(fd, LOCK_UN);
```

## 2 레코드 잠금 (Record Locking)

### 개요

* 파일 전체가 아닌 특정 바이트 범위(레코드)를 잠금.
* `fcntl()` 함수 사용.

### flock 구조체

```c
struct flock {
  short l_type;     // F_RDLCK, F_WRLCK, F_UNLCK
  short l_whence;   // SEEK_SET 등
  off_t l_start;    // 시작 위치
  off_t l_len;      // 길이
  pid_t l_pid;      // 프로세스 ID
};
```

### 예제 코드 (읽기 잠금)

```c
lock.l_type = F_RDLCK;
lock.l_whence = SEEK_SET;
lock.l_start = (id-START_ID)*sizeof(record);
lock.l_len = sizeof(record);
fcntl(fd, F_SETLKW, &lock);
```

### 예제 코드 (쓰기 잠금)

```c
lock.l_type = F_WRLCK;
fcntl(fd, F_SETLKW, &lock);
// 레코드 수정 후
lock.l_type = F_UNLCK;
fcntl(fd, F_SETLK, &lock);
```

## 3 간단한 잠금 함수 lockf()

### 주요 명령어

* `F_LOCK`: 쓰기 잠금 (블로킹)
* `F_TLOCK`: 쓰기 잠금 (논블로킹)
* `F_ULOCK`: 잠금 해제
* `F_TEST`: 잠금 여부 검사

```c
#include <unistd.h>
int lockf(int fd, int cmd, off_t len);
```

## 4 권고 잠금 vs 강제 잠금

### 권고 잠금 (Advisory Lock)

* 잠금 규칙을 프로세스들이 **자발적으로 준수**해야 함.
* 대부분의 UNIX/Linux 시스템이 기본적으로 사용함.

### 강제 잠금 (Mandatory Lock)

* 시스템이 강제로 잠금 규칙을 **강제 적용**.
* 리눅스에서는 마운트 옵션(`-o mand`) 필요.
* `chmod 2644 filename` 으로 파일에 SGID 비트 설정 및 그룹 실행 비트 제거 필요.

---

## 핵심 요약

* `flock()`: 파일 전체 잠금 (간단)
* `fcntl()`: 레코드 단위 잠금 (정교함)
* `lockf()`: 간단한 레코드 잠금 함수
* `권고 잠금`: 프로세스 간 약속 기반
* `강제 잠금`: 커널이 강제, 성능 저하 가능

---
