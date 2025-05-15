
# 📒 시스템 프로그래밍
---

## 파일 디스크립터 복제 (dup, dup2)

### ▷ dup(oldfd)
- 기존 파일 디스크립터를 복제.
- 새로운 파일 디스크립터 반환.
- 동일한 열린 파일 테이블 엔트리를 공유.

### ▷ dup2(oldfd, newfd)
- `oldfd`를 `newfd`에 복제.
- `newfd`가 열린 상태면 먼저 닫고 복제.

### ✅ 특징
- 복제된 디스크립터는 동일한 파일 위치 포인터 사용.
- 즉, 읽기/쓰기 위치 공유.
- 상태 플래그도 공유됨.

### 💡 실습 예제 (dup.c)
```
fd1 = creat("myfile", 0600);
write(fd1, "Hello! Linux", 12);
fd2 = dup(fd1);
write(fd2, "Bye! Linux", 10);
```
📎 결과
```
Hello! LinuxBye! Linux
```
## 파일 시스템 구조와 상태 정보
### ▷ 파일 시스템 구성 요소
구성 요소	설명
Boot block	부트 코드
Super block	메타 정보 (총 블록 수, i-노드 수)
i-list	i-노드 리스트
Data block	실제 데이터 저장 블록

### ▷ i-노드 (inode)
파일의 메타 정보 저장.

파일당 1개.

파일 타입, 권한, 크기, 소유자, 시간, 블록 포인터 등 포함.

### ▷ 파일 상태 확인 API
```
stat("파일경로", &buf);
fstat(fd, &buf);
lstat("심볼릭 링크", &buf);
```
### ▷ 파일 타입 확인 매크로
타입	매크로
일반파일	S_ISREG()
디렉터리	S_ISDIR()
문자장치	S_ISCHR()
블록장치	S_ISBLK()
FIFO	S_ISFIFO()
소켓	S_ISSOCK()
심볼릭링크	S_ISLNK()

### ▷ 파일 권한 변경
```
chmod("파일명", 0644);
```
💡 이해 요약
- 파일의 상태와 권한은 i-노드를 통해 관리됨.

- stat() 호출 시 i-노드 정보를 읽어옴.

- ls -l 은 이 정보를 활용해 화면에 출력함.

## 디렉터리 리스트 프로그램 (list2.c)
### ▷ 목적
ls -l 명령어처럼 디렉터리 내 파일 상태를 상세히 출력.

### ▷ 주요 함수
함수명	기능
opendir	디렉터리 열기
readdir	디렉터리 엔트리 읽기
lstat	파일 상태 정보 획득
printStat	파일 상태 정보 출력 (타입, 권한, 소유자, 크기 등)

### ▷ 핵심 흐름
1. opendir() 으로 디렉터리 열기.

2. readdir() 로 파일 하나씩 읽기.

3. lstat() 로 상태 확인.

4. printStat() 호출하여 상세 정보 출력.

5. 종료 시 closedir().

### ▷ 파일 상태 출력 함수
```
void printStat(char *pathname, char *file, struct stat *st) {
    printf("%5d ", st->st_blocks);
    printf("%c%s ", type(st->st_mode), perm(st->st_mode));
    printf("%3d ", st->st_nlink);
    printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name);
    printf("%9d ", st->st_size);
    printf("%.12s ", ctime(&st->st_mtime)+4);
    printf("%s\n", file);
}
```
##💡 이해 요약
- 파일의 상태 정보 (st_blocks, st_mode, st_size 등)를 읽어서 상세히 출력.

- type() 함수로 파일 타입 반환.

- perm() 함수로 권한 문자열 생성.

##🔑 전체 요약
장	핵심 요약
5	dup(), dup2() → 파일 디스크립터 복제, 동일한 파일 포인터 공유
6	i-노드를 통한 파일 상태 관리, stat(), chmod() 사용 가능
7	디렉터리 리스트 상세 출력 (ls -l 직접 구현)

