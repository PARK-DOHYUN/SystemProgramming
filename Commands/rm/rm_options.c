#include "rm_options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * 옵션 구조체 초기화
 * @param opts 초기화할 옵션 구조체 포인터
 * 
 * 모든 옵션을 기본값(false)으로 설정하여 초기화
 * 프로그램 시작시 반드시 호출되어야 하는 함수
 */
void init_options(rm_options_t *opts) {
    opts->recursive = false;     // -r: 재귀 삭제 비활성화
    opts->force = false;         // -f: 강제 삭제 비활성화  
    opts->interactive = false;   // -i: 대화형 모드 비활성화
    opts->verbose = false;       // -v: 상세 출력 비활성화
    opts->zero_only = false;     // -z: 0바이트 파일만 삭제 비활성화
}

/**
 * 사용법 출력
 * @param program_name 프로그램 이름 (argv[0])
 * 
 * 도움말 요청시나 잘못된 사용시 표준 출력으로 사용법을 출력
 * GNU rm과 유사한 형태의 도움말 제공
 */
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTION]... FILE...\n", program_name);
    printf("Remove (unlink) the FILE(s).\n\n");
    printf("Options:\n");
    printf("  -r, --recursive   remove directories and their contents recursively\n");
    printf("  -f, --force       ignore nonexistent files, never prompt\n");
    printf("  -i, --interactive prompt before every removal\n");
    printf("  -v, --verbose     explain what is being done\n");
    printf("  -z, --zero        remove only zero-byte files\n");
    printf("\nOptions can be combined (e.g., -rzv)\n");
}

/**
 * 단일 옵션 문자 처리
 * @param opt 옵션 문자 ('r', 'f', 'i', 'v', 'z')
 * @param opts 설정할 옵션 구조체 포인터
 * 
 * 단축 옵션(-r, -f 등)을 처리하여 해당하는 옵션 구조체 멤버를 활성화
 * 잘못된 옵션이 입력되면 에러 메시지 출력 후 프로그램 종료
 */
static void process_option_char(char opt, rm_options_t *opts) {
    switch (opt) {
        case 'r':
            opts->recursive = true;    // 재귀 삭제 활성화
            break;
        case 'f':
            opts->force = true;        // 강제 삭제 활성화
            break;
        case 'i':
            opts->interactive = true;  // 대화형 모드 활성화
            break;
        case 'v':
            opts->verbose = true;      // 상세 출력 활성화
            break;
        case 'z':
            opts->zero_only = true;    // 0바이트 파일만 삭제 활성화
            break;
        default:
            // 지원하지 않는 옵션이 입력된 경우
            fprintf(stderr, "Invalid option: -%c\n", opt);
            exit(1);  // 즉시 프로그램 종료
    }
}

/**
 * 옵션 파싱 함수
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @param opts 파싱 결과를 저장할 옵션 구조체 포인터
 * @param file_start_index 파일 인자가 시작되는 인덱스를 저장할 포인터
 * @return 0: 성공, -1: 실패
 * 
 * 명령행에서 전달된 옵션들을 파싱하여 옵션 구조체에 설정
 * 단축 옵션(-r), 긴 옵션(--recursive), 묶음 옵션(-rfv) 모두 지원
 */
int parse_options(int argc, char *argv[], rm_options_t *opts, int *file_start_index) {
    int i;
    
    // 모든 명령행 인자를 순회하며 옵션 처리
    for (i = 1; i < argc; i++) {
        // '-'로 시작하지 않으면 파일 인자로 간주하고 옵션 파싱 종료
        if (argv[i][0] != '-') {
            break;
        }
        
        // "--" 구분자를 만나면 이후는 모두 파일 인자로 처리
        // 예: rm -- -filename (파일명이 -로 시작하는 경우)
        if (strcmp(argv[i], "--") == 0) {
            i++;  // "--" 다음 인덱스부터 파일 인자
            break;
        }
        
        // 긴 옵션 처리 (--로 시작)
        if (strncmp(argv[i], "--", 2) == 0) {
            char *long_opt = argv[i] + 2;  // "--" 이후 문자열
            
            if (strcmp(long_opt, "recursive") == 0) {
                opts->recursive = true;
            } else if (strcmp(long_opt, "force") == 0) {
                opts->force = true;
            } else if (strcmp(long_opt, "interactive") == 0) {
                opts->interactive = true;
            } else if (strcmp(long_opt, "verbose") == 0) {
                opts->verbose = true;
            } else if (strcmp(long_opt, "zero") == 0) {
                opts->zero_only = true;
            } else if (strcmp(long_opt, "help") == 0) {
                // 도움말 요청시 사용법 출력 후 정상 종료
                print_usage(argv[0]);
                exit(0);
            } else {
                // 지원하지 않는 긴 옵션
                fprintf(stderr, "Invalid option: %s\n", argv[i]);
                return -1;
            }
        }
        // 짧은 옵션 처리 (-로 시작, 묶음 옵션 지원)
        else {
            char *opt_str = argv[i] + 1;  // '-' 건너뛰기
            
            // 묶음 옵션의 각 문자를 개별적으로 처리
            // 예: -rfv는 -r, -f, -v로 각각 처리됨
            for (int j = 0; opt_str[j] != '\0'; j++) {
                if (opt_str[j] == 'h') {
                    // 도움말 요청시 사용법 출력 후 정상 종료
                    print_usage(argv[0]);
                    exit(0);
                }
                // 각 옵션 문자를 개별적으로 처리
                process_option_char(opt_str[j], opts);
            }
        }
    }
    
    // 파일 인자가 시작되는 인덱스를 반환 인자에 저장
    *file_start_index = i;
    return 0;  // 성공
}