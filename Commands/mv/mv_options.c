#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "mv_options.h"

/**
 * mv_options_t 구조체의 모든 필드를 기본값으로 초기화하는 함수
 * @param opts 초기화할 옵션 구조체 포인터
 * 
 * 모든 불린 플래그를 false로 설정하여 기본 동작 상태로 만든다.
 * 이는 mv 명령어의 기본 동작이 덮어쓰기를 허용하는 것과 일치한다.
 */
void init_options(mv_options_t *opts) {
    opts->interactive = false;  // -i 옵션: 덮어쓰기 전 사용자 확인 비활성화
    opts->force = false;        // -f 옵션: 강제 덮어쓰기 비활성화
    opts->no_clobber = false;   // -n 옵션: 덮어쓰기 방지 비활성화
    opts->verbose = false;      // -v 옵션: 상세 출력 비활성화
    opts->suffix = false;       // -s 옵션: 중복 시 숫자 추가 비활성화
}

/**
 * 명령줄 인자에서 옵션들을 파싱하는 함수
 * @param argc 명령줄 인자 개수
 * @param argv 명령줄 인자 배열
 * @param opts 파싱된 옵션을 저장할 구조체 포인터
 * @return 성공시 옵션이 아닌 첫 번째 인자의 인덱스, 실패시 -1
 * 
 * getopt() 함수를 사용하여 POSIX 표준 옵션 파싱을 수행한다.
 * 지원하는 옵션: -i, -f, -n, -v, -s
 * -f와 -n은 상호 배타적이며, 동시에 사용될 경우 -f가 우선한다.
 */
int parse_options(int argc, char *argv[], mv_options_t *opts) {
    int opt;  // getopt()가 반환하는 현재 처리 중인 옵션 문자
    
    // getopt()를 사용하여 옵션 문자열 "ifnvs"에 정의된 옵션들을 순차 처리
    // 각 문자는 하나의 옵션을 나타내며, ':'이 붙으면 인자가 필요함을 의미
    while ((opt = getopt(argc, argv, "ifnvs")) != -1) {
        switch (opt) {
            case 'i':
                // -i: interactive mode - 덮어쓰기 전 사용자에게 확인 요청
                opts->interactive = true;
                break;
            case 'f':
                // -f: force - 강제 덮어쓰기, 경고나 확인 없이 진행
                opts->force = true;
                break;
            case 'n':
                // -n: no-clobber - 기존 파일을 덮어쓰지 않음
                opts->no_clobber = true;
                break;
            case 'v':
                // -v: verbose - 이동 과정의 상세 정보 출력
                opts->verbose = true;
                break;
            case 's':
                // -s: suffix - 중복 파일명 시 숫자를 추가하여 고유한 이름 생성
                opts->suffix = true;
                break;
            case '?':
                // 알 수 없는 옵션이거나 잘못된 옵션 사용 시
                fprintf(stderr, "사용법: mv [-ifnvs] 원본 대상\n");
                return -1;
        }
    }
    
    // -f(force)와 -n(no-clobber)은 서로 반대되는 기능이므로 상호 배타적
    // 둘 다 지정된 경우 GNU mv와 동일하게 -f가 우선하도록 처리
    if (opts->force && opts->no_clobber) {
        opts->no_clobber = false; // -f가 우선하므로 -n 무효화
    }
    
    // optind는 getopt()가 설정하는 전역 변수로,
    // 다음에 처리할 argv 인덱스 (즉, 옵션이 아닌 첫 번째 인자)를 가리킴
    return optind;
}

/**
 * 파일 덮어쓰기 여부를 결정하는 함수
 * @param dest 대상 파일 경로
 * @param opts 명령줄 옵션 구조체 포인터
 * @return 덮어쓰기 허용시 true, 거부시 false
 * 
 * 옵션에 따른 덮어쓰기 정책:
 * 1. 대상 파일이 존재하지 않으면 무조건 허용
 * 2. -n 옵션: 무조건 거부
 * 3. -f 옵션: 무조건 허용
 * 4. -i 옵션: 사용자에게 확인
 * 5. 기본값: 허용 (전통적인 mv 동작)
 */
bool should_overwrite(const char *dest, mv_options_t *opts) {
    struct stat st;  // 파일 상태 정보를 저장할 구조체
    
    // stat()로 대상 파일의 존재 여부 확인
    // 파일이 존재하지 않으면 (stat 실패) 덮어쓰기 문제가 없으므로 허용
    if (stat(dest, &st) != 0) {
        return true;
    }
    
    // -n 옵션: no-clobber, 기존 파일을 절대 덮어쓰지 않음
    if (opts->no_clobber) {
        return false;
    }
    
    // -f 옵션: force, 어떤 경우에도 강제로 덮어쓰기
    // 파일 권한이나 다른 제약과 관계없이 시도
    if (opts->force) {
        return true;
    }
    
    // -i 옵션: interactive, 사용자에게 덮어쓰기 여부를 직접 확인
    if (opts->interactive) {
        char response;  // 사용자 입력을 저장할 변수
        printf("'%s'를 덮어쓰시겠습니까? (y/n): ", dest);
        scanf(" %c", &response);  // 앞의 공백 문자는 이전 입력의 개행문자 무시
        // 'y' 또는 'Y' 입력시에만 덮어쓰기 허용
        return (response == 'y' || response == 'Y');
    }
    
    // 어떤 옵션도 지정되지 않은 경우의 기본 동작
    // 전통적인 mv 명령어는 확인 없이 덮어쓰기를 수행
    return true;
}

/**
 * 중복 파일명이 있을 때 고유한 파일명을 생성하는 함수 (-s 옵션용)
 * @param dest 원래 대상 파일 경로
 * @return 고유한 파일명이 담긴 새로 할당된 문자열 (호출자가 free 필요)
 * 
 * 알고리즘:
 * 1. 원본 파일명이 존재하지 않으면 그대로 반환
 * 2. 파일명을 기본 이름과 확장자로 분리
 * 3. 기본이름_숫자.확장자 형태로 고유한 이름을 찾을 때까지 시도
 * 4. 예: file.txt -> file_1.txt, file_2.txt, ...
 */
char* generate_unique_name(const char *dest) {
    struct stat st;         // 파일 존재 확인용 구조체
    char *new_name;         // 생성된 새 파일명을 저장할 포인터
    char *base_name, *extension;  // 기본 파일명과 확장자
    char *dot_pos;          // 확장자 구분점(마지막 '.')의 위치
    int counter = 1;        // 고유 번호 생성용 카운터
    
    // 원본 파일명이 존재하지 않으면 중복이 아니므로 그대로 사용 가능
    if (stat(dest, &st) != 0) {
        new_name = malloc(strlen(dest) + 1);  // null terminator 포함
        strcpy(new_name, dest);
        return new_name;
    }
    
    // 파일명과 확장자를 분리하기 위해 원본 문자열 복사
    base_name = malloc(strlen(dest) + 1);
    strcpy(base_name, dest);
    
    // strrchr()로 마지막 '.' 위치를 찾아 확장자 분리
    // 예: "file.txt" -> base_name="file", extension="txt"
    dot_pos = strrchr(base_name, '.');
    if (dot_pos != NULL) {
        *dot_pos = '\0';        // '.' 위치에 null 문자 삽입하여 기본명 분리
        extension = dot_pos + 1; // '.' 다음부터가 확장자
    } else {
        // 확장자가 없는 파일 (예: "README")
        extension = "";
    }
    
    // 고유한 이름을 찾기 위한 충분한 메모리 할당
    // 원본 길이 + 숫자 부분 + 구분자들을 위한 여유 공간
    new_name = malloc(strlen(dest) + 20);
    
    // 중복되지 않는 파일명을 찾을 때까지 반복
    do {
        if (strlen(extension) > 0) {
            // 확장자가 있는 경우: base_name_counter.extension
            snprintf(new_name, strlen(dest) + 20, "%s_%d.%s", base_name, counter, extension);
        } else {
            // 확장자가 없는 경우: base_name_counter
            snprintf(new_name, strlen(dest) + 20, "%s_%d", base_name, counter);
        }
        counter++;  // 다음 시도를 위해 카운터 증가
    } while (stat(new_name, &st) == 0);  // 파일이 존재하는 동안 계속 반복
    
    // 임시로 할당한 base_name 메모리 해제
    free(base_name);
    return new_name;  // 호출자가 free()로 해제해야 함
}