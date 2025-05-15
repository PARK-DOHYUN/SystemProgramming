#include <sys/types.h>  // 데이터 타입 정의 (예: mode_t)
#include <sys/stat.h>   // 파일 상태 정보를 위한 구조체 및 함수
#include <dirent.h>     // 디렉터리 관련 함수 (예: opendir, readdir, closedir)
#include <pwd.h>        // 사용자 정보를 얻기 위한 함수 (예: getpwuid)
#include <grp.h>        // 그룹 정보를 얻기 위한 함수 (예: getgrgid)
#include <stdio.h>      // 표준 입출력 함수 (예: printf, perror)
#include <stdlib.h>  // exit() 함수 사용을 위한 헤더
#include <time.h>    // ctime() 함수 사용을 위한 헤더

char type(mode_t);      // 파일 타입을 반환하는 함수
char *perm(mode_t);     // 파일 권한을 반환하는 함수
void printStat(char*, char*, struct stat*);  // 파일 상태 정보를 출력하는 함수

/* 디렉터리 내용을 자세히 리스트한다. */
int main(int argc, char **argv) {
    DIR *dp;  // 디렉터리 스트림을 위한 포인터
    char *dir;  // 디렉터리 경로를 저장할 문자열
    struct stat st;  // 파일 상태 정보를 저장할 구조체
    struct dirent *d;  // 디렉터리 엔트리 정보
    char path[BUFSIZ+1];  // 파일 경로를 저장할 버퍼
    
    // 명령행 인자에서 디렉터리 경로를 받지 않으면 현재 디렉터리(`.`)로 설정
    if (argc == 1)
        dir = ".";
    else
        dir = argv[1];

    // 디렉터리 열기
    if ((dp = opendir(dir)) == NULL)
        perror(dir);  // 디렉터리 열기 실패 시 오류 출력

    // 디렉터리 내의 각 파일에 대해 처리
    while ((d = readdir(dp)) != NULL) {  // 디렉터리에서 파일 읽기
        sprintf(path, "%s/%s", dir, d->d_name);  // 파일 경로명 만들기
        if (lstat(path, &st) < 0)  // 파일 상태 정보 가져오기
            perror(path);  // 상태 정보 가져오기 실패 시 오류 출력
        printStat(path, d->d_name, &st);  // 파일 상태 정보 출력
        putchar('\n');  // 출력 후 한 줄 띄우기
    }

    closedir(dp);  // 디렉터리 스트림 닫기
    exit(0);  // 프로그램 종료
}

/* 파일 상태 정보를 출력하는 함수 */
void printStat(char *pathname, char *file, struct stat *st) {
    // 파일의 블록 크기 출력
    printf("%5ld ", st->st_blocks);
    // 파일 타입과 권한 출력
    printf("%c%s ", type(st->st_mode), perm(st->st_mode));
    // 하드 링크 수 출력
    printf("%3ld ", st->st_nlink);
    // 파일 소유자 및 그룹 출력
    printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name);
    // 파일 크기 출력
    printf("%9ld ", st->st_size);
    // 마지막 수정 시간 출력 (날짜 형식)
    printf("%.12s ", ctime(&st->st_mtime)+4);
    // 파일 이름 출력
    printf("%s", file);
}

/* 파일 타입을 리턴하는 함수 */
char type(mode_t mode) {
    // 파일 타입에 따라 문자 반환
    if (S_ISREG(mode))  // 일반 파일
        return('-');
    if (S_ISDIR(mode))  // 디렉터리
        return('d');
    if (S_ISCHR(mode))  // 문자 장치 파일
        return('c');
    if (S_ISBLK(mode))  // 블록 장치 파일
        return('b');
    if (S_ISLNK(mode))  // 심볼릭 링크
        return('l');
    if (S_ISFIFO(mode))  // FIFO (명명된 파이프)
        return('p');
    if (S_ISSOCK(mode))  // 소켓 파일
        return('s');
}

/* 파일 권한을 리턴하는 함수 */
char* perm(mode_t mode) {
    int i;
    static char perms[10] = "---------";  // 기본 권한 문자열 초기화

    // 3개의 그룹(소유자, 그룹, 기타 사용자)에 대해 권한을 설정
    for (i = 0; i < 3; i++) {
        if (mode & (S_IREAD >> i*3))  // 읽기 권한
            perms[i*3] = 'r';
        if (mode & (S_IWRITE >> i*3))  // 쓰기 권한
            perms[i*3+1] = 'w';
        if (mode & (S_IEXEC >> i*3))  // 실행 권한
            perms[i*3+2] = 'x';
    }
    return(perms);  // 권한 문자열 반환
}
