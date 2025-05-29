/**
 * cp_options.c - cp 명령어 옵션 처리 구현
 * 
 * 이 파일은 cp 명령어의 옵션 파싱과 초기화,
 * 도움말 출력 기능을 구현합니다.
 */

#include "cp_options.h"
#include <stdio.h>      // 표준 입출력 (printf, fprintf)
#include <string.h>     // 문자열 처리 (strlen)
#include <stdlib.h>     // 일반 유틸리티 (사용되지 않음, 확장용)

/**
 * init_options - cp_options_t 구조체를 기본값으로 초기화
 * @opts: 초기화할 옵션 구조체 포인터
 * 
 * 모든 옵션을 비활성화 상태(false)로 설정합니다.
 * cp 프로그램 시작 시 반드시 호출되어야 합니다.
 */
void init_options(cp_options_t *opts) {
    opts->interactive = false;  // -i 옵션: 덮어쓰기 확인 비활성화
    opts->force = false;        // -f 옵션: 강제 복사 비활성화
    opts->update = false;       // -u 옵션: 조건부 복사 비활성화
    opts->preserve = false;     // -p 옵션: 속성 보존 비활성화
}

/**
 * print_usage - 프로그램 사용법을 표준 출력으로 출력
 * @program_name: 프로그램 이름 (보통 argv[0])
 * 
 * 잘못된 옵션 사용이나 인자 부족 시 호출되어
 * 올바른 사용법과 사용 가능한 옵션들을 안내합니다.
 */
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] SOURCE DEST\n", program_name);
    printf("Copy SOURCE to DEST\n\n");
    printf("Options:\n");
    printf("  -i    prompt before overwrite\n");                              // 덮어쓰기 전 확인
    printf("  -f    force copy (remove existing destination files)\n");       // 강제 복사
    printf("  -u    copy only when SOURCE is newer than DEST\n");             // 조건부 복사
    printf("  -p    preserve file attributes (permissions, ownership, timestamps)\n"); // 속성 보존
    printf("\nOptions can be combined: -ifu, -ip, etc.\n");                   // 옵션 결합 가능
}

/**
 * parse_options - 명령행 인자를 파싱하여 옵션과 파일 경로를 추출
 * @argc: main 함수의 argc (명령행 인자 개수)
 * @argv: main 함수의 argv (명령행 인자 배열)
 * @opts: 파싱된 옵션을 저장할 구조체 포인터
 * @src: 소스 파일 경로를 저장할 문자열 포인터의 포인터
 * @dst: 대상 파일 경로를 저장할 문자열 포인터의 포인터
 * 
 * 지원하는 옵션:
 * - -i: 덮어쓰기 전 사용자 확인
 * - -f: 강제 복사 (기존 파일 강제 삭제)
 * - -u: 소스가 더 새로운 경우에만 복사
 * - -p: 파일 속성 보존 (권한, 소유자, 시간)
 * 
 * 옵션은 묶어서 사용 가능: -ifu, -ip 등
 * 
 * @return: 성공 시 0, 실패 시 -1
 */
int parse_options(int argc, char *argv[], cp_options_t *opts, char **src, char **dst) {
    int arg_index = 1;  // 현재 처리 중인 인자의 인덱스 (argv[0]은 프로그램명이므로 1부터 시작)
    
    // 명령행 인자를 순서대로 검사하여 옵션 추출
    while (arg_index < argc && argv[arg_index][0] == '-') {
        char *option = argv[arg_index] + 1; // '-' 문자를 건너뛰고 옵션 문자들 시작
        
        // 단순히 '-'만 있는 경우 (잘못된 사용)
        if (strlen(option) == 0) {
            fprintf(stderr, "Invalid option: -\n");
            return -1;
        }
        
        // 묶음 옵션 처리 (예: -ifu는 -i, -f, -u를 모두 의미)
        for (int i = 0; option[i] != '\0'; i++) {
            switch (option[i]) {
                case 'i':
                    opts->interactive = true;   // 덮어쓰기 확인 활성화
                    break;
                case 'f':
                    opts->force = true;         // 강제 복사 활성화
                    break;
                case 'u':
                    opts->update = true;        // 조건부 복사 활성화
                    break;
                case 'p':
                    opts->preserve = true;      // 속성 보존 활성화
                    break;
                default:
                    // 알 수 없는 옵션 문자
                    fprintf(stderr, "Unknown option: -%c\n", option[i]);
                    return -1;
            }
        }
        arg_index++; // 다음 인자로 이동
    }
    
    // 옵션 파싱 완료 후 남은 인자 개수 확인
    // 소스 파일과 대상 파일 정확히 2개가 있어야 함
    if (argc - arg_index < 2) {
        fprintf(stderr, "Error: Missing source or destination file\n");
        print_usage(argv[0]);
        return -1;
    }
    
    if (argc - arg_index > 2) {
        fprintf(stderr, "Error: Too many arguments\n");
        print_usage(argv[0]);
        return -1;
    }
    
    // 소스 파일과 대상 파일 경로 설정
    *src = argv[arg_index];     // 첫 번째 파일 인자는 소스
    *dst = argv[arg_index + 1]; // 두 번째 파일 인자는 대상
    
    return 0; // 성공적으로 파싱 완료
}