# 📘 리눅스 시스템 프로그래밍 - 파일 입출력 & 매직 넘버

이 노트는 리눅스 시스템 프로그래밍 수업의 파일 입출력 예제와 리눅스에서 파일 확장자 없이 파일 형식을 식별하는 방법을 정리한 자료입니다.

---

## 📂 시스템 호출 개요

리눅스에서는 프로그램이 커널에 서비스를 요청할 때 시스템 호출을 사용합니다.

| 자원 유형 | 주요 시스템 호출                                                      |
| ----- | -------------------------------------------------------------- |
| 파일    | `open()`, `read()`, `write()`, `close()`, `lseek()`, `dup()` 등 |
| 프로세스  | `fork()`, `exec()`, `exit()`, `wait()` 등                       |
| 메모리   | `malloc()`, `calloc()`, `free()` 등 (실제로는 사용자 영역에서 호출)          |
| 시그널   | `signal()`, `kill()`, `alarm()` 등                              |
| IPC   | `pipe()`, `socket()` 등                                         |

---

## 📁 파일 다루기: 시스템 호출 정리

### `open()`

```c
int open(const char *path, int oflag, [mode_t mode]);
```

* 파일 열기
* 성공 시 파일 디스크립터 반환, 실패 시 `-1`

**주요 oflag 플래그**:

| 플래그        | 설명                    |
| ---------- | --------------------- |
| `O_RDONLY` | 읽기 전용                 |
| `O_WRONLY` | 쓰기 전용                 |
| `O_RDWR`   | 읽기/쓰기                 |
| `O_CREAT`  | 파일 없으면 생성 (`mode` 필요) |
| `O_TRUNC`  | 기존 파일 내용 삭제           |
| `O_APPEND` | 파일 끝에 덧붙이기            |
| `O_EXCL`   | 이미 존재 시 실패            |

---

### `fopen.c` 예제

```c
if ((fd = open(argv[1], O_RDWR)) == -1)
    printf("파일 열기 오류\n");
else
    printf("파일 열기 성공\n");
```

---

### `creat()`

```c
int creat(const char *path, mode_t mode);
```

* 파일을 생성하고 쓰기 전용으로 열며, 내부적으로 `open(path, O_WRONLY | O_CREAT | O_TRUNC, mode)`와 동일

---

### `close()`

```c
int close(int fd);
```

* 열린 파일 디스크립터를 닫음

---

### `read()`

```c
ssize_t read(int fd, void *buf, size_t nbytes);
```

* 파일에서 데이터를 읽어 버퍼에 저장

---

### `write()`

```c
ssize_t write(int fd, void *buf, size_t nbytes);
```

* 버퍼의 데이터를 파일에 씀

---

## 📋 `copy.c`: 파일 복사 프로그램

```c
int fd1, fd2, n;
char buf[BUFSIZ];

if ((fd1 = open(argv[1], O_RDONLY)) == -1) ...
if ((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) ...

while ((n = read(fd1, buf, BUFSIZ)) > 0)
    write(fd2, buf, n);

close(fd1); close(fd2);
```

* 사용법: `./copy 원본 대상`
* `BUFSIZ`만큼 데이터를 읽고 대상 파일에 씀

---

## 🧪 리눅스에서 파일 확장자 없이 파일 형식 인식하기

### 확장자 vs 매직 넘버

| 기준     | 확장자 (`.jpg`) | 매직 넘버 (Magic Number) |
| ------ | ------------ | -------------------- |
| 형태     | 파일명 일부       | 파일 시작 바이트            |
| 시스템 인식 | X (사람만 참고)   | ✅ (리눅스 시스템이 실제로 사용)  |
| 변경 영향  | 무의미          | 내용이 다르면 오작동 가능       |

---

### 매직 넘버 예시

| 파일 형식 | 매직 넘버 (16진수)              | 설명          |
| ----- | ------------------------- | ----------- |
| PNG   | `89 50 4E 47 0D 0A 1A 0A` | PNG 이미지     |
| JPG   | `FF D8 FF`                | JPEG 이미지 시작 |
| PDF   | `25 50 44 46`             | `%PDF`      |
| ELF   | `7F 45 4C 46`             | 리눅스 실행 파일   |
| ZIP   | `50 4B 03 04`             | ZIP 압축      |

---

### `file` 명령어로 매직 넘버 확인

```bash
$ file image.png
image.png: PNG image data
```

```bash
$ xxd -l 8 image.png
00000000: 8950 4e47 0d0a 1a0a                      .PNG....
```

---

## ⚠️ 확장자 우회와 보안

* 리눅스에서는 확장자를 조작해도 시스템은 파일 내용을 보고 판단
* 예: `virus.jpg.exe` → 확장자보다 **실제 내용(매직 넘버)** 이 중요

---

## 📚 참고 자료

* 리눅스 프로그래밍 (창병모, 생능출판)
* [https://www.44bits.io/ko/post/wsl2-install-and-basic-usage](https://www.44bits.io/ko/post/wsl2-install-and-basic-usage)
