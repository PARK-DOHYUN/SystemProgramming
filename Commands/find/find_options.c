/*
 * find_options.c - find 명령어 옵션 파싱 모듈
 * 
 * 이 파일은 명령행 인자를 파싱하여 검색 옵션을 설정하는 기능을 제공합니다.
 * Unix find 명령어와 유사한 옵션들을 지원하며, 단축형 옵션을 묶어서 사용할 수 있습니다.
 * 
 * 지원 옵션:
 * -f: 파일만 검색
 * -d: 디렉토리만 검색  
 * -e: 빈 파일/디렉토리만 검색
 * -n [패턴]: 이름 패턴 매칭 (대소문자 구분)
 * -i [패턴]: 이름 패턴 매칭 (대소문자 무시)
 * -s [크기]: 파일 크기 조건
 * -u [사용자]: 소유자 조건
 * -p [권한]: 권한 조건
 * -t [일수]: 수정 시간 조건 (+n: n일 이전, -n: n일 이내, n: 정확히 n일 전)
 */

#include "find_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 옵션 구조체를 기본값으로 초기화
 * 
 * @param opts 초기화할 옵션 구조체 포인터
 */
void init_options(find_options_t *opts) {
    // 구조체 전체를 0으로 초기화 (모든 불린 값은 false, 포인터는 NULL)
    memset(opts, 0, sizeof(find_options_t));
    
    // 기본 검색 경로는 현재 디렉토리
    opts->search_path = strdup(".");
    
    // mtime 관련 기본값 설정
    opts->mtime_days = -1;      // 초기값은 -1 (설정되지 않음을 의미)
    opts->mtime_prefix = 0;     // 접두어 없음
    opts->mtime_set = false;    // mtime 조건이 설정되지 않음
}

/**
 * 동적 할당된 옵션 메모리 해제
 * 
 * @param opts 메모리를 해제할 옵션 구조체 포인터
 */
void free_options(find_options_t *opts) {
    // 각 문자열 포인터가 NULL이 아니면 메모리 해제
    if (opts->name_pattern) free(opts->name_pattern);
    if (opts->iname_pattern) free(opts->iname_pattern);
    if (opts->size_spec) free(opts->size_spec);
    if (opts->user_name) free(opts->user_name);
    if (opts->perm_spec) free(opts->perm_spec);
    if (opts->search_path) free(opts->search_path);
}

/**
 * 사용법 출력
 * 
 * @param prog_name 프로그램 이름
 */
void print_usage(const char *prog_name) {
    printf("사용법: %s [경로] [옵션들]\n", prog_name);
    printf("옵션:\n");
    printf("  -f           파일만 검색 (-type f)\n");
    printf("  -d           디렉토리만 검색 (-type d)\n");
    printf("  -e           빈 파일/디렉토리만 검색 (-empty)\n");
    printf("  -n [이름]    이름으로 검색 (-name)\n");
    printf("  -i [이름]    이름으로 검색, 대소문자 무시 (-iname)\n");
    printf("  -s [크기]    크기로 검색 (-size), +/-접두어 사용\n");
    printf("  -u [사용자]  사용자로 검색 (-user)\n");
    printf("  -p [권한]    권한으로 검색 (-perm)\n");
    printf("  -t [일수]    수정 시간으로 검색 (-mtime)\n");
    printf("               +n: n일 이전, -n: n일 이내, n: 정확히 n일 전\n");
    printf("\n");
    printf("예시:\n");
    printf("  %s -f -n \"*.c\" -e     # 빈 .c 파일 검색\n", prog_name);
    printf("  %s -fde -n main.c     # 묶음 옵션 사용\n", prog_name);
    printf("  %s -t +7              # 7일 이전에 수정된 파일\n", prog_name);
    printf("  %s -t -3              # 3일 이내에 수정된 파일\n", prog_name);
    printf("  %s -t 5               # 정확히 5일 전에 수정된 파일\n", prog_name);
}

/**
 * 명령행 인자를 파싱하여 옵션 구조체에 설정
 * 묶음 옵션(-fde 같은 형태)과 개별 옵션을 모두 지원
 * 
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @param opts 파싱 결과를 저장할 옵션 구조체
 * @return 성공시 0, 도움말 출력시 1, 오류시 -1
 */
int parse_options(int argc, char *argv[], find_options_t *opts) {
    // 첫 번째 인자가 경로인지 확인 (옵션 플래그('-')로 시작하지 않는 경우)
    if (argc > 1 && argv[1][0] != '-') {
        // 기존 기본 경로 메모리 해제
        free(opts->search_path);
        // 새로운 검색 경로 설정
        opts->search_path = strdup(argv[1]);
        
        // argv 배열을 재정렬하여 경로 인자를 제거
        // 이후 옵션 파싱에서 경로가 방해되지 않도록 함
        for (int i = 1; i < argc - 1; i++) {
            argv[i] = argv[i + 1];
        }
        argc--;  // 인자 개수 감소
    }
    
    // 수동 파싱 (묶음 옵션 지원을 위해)
    // 예: -fde 같은 형태로 여러 옵션을 한번에 지정 가능
    for (int i = 1; i < argc; i++) {
        // 옵션이 아닌 인자는 건너뛰기
        if (argv[i][0] != '-') {
            continue;
        }
        
        char *arg = argv[i] + 1;  // '-' 문자 제거하고 옵션 문자들만
        
        // 옵션 문자열의 각 문자를 순회하며 처리
        for (int j = 0; arg[j] != '\0'; j++) {
            switch (arg[j]) {
                case 'f':
                    opts->type_file = true;  // 파일만 검색
                    break;
                    
                case 'd':
                    opts->type_dir = true;   // 디렉토리만 검색
                    break;
                    
                case 'e':
                    opts->empty_filter = true;  // 빈 파일/디렉토리만
                    break;
                    
                case 'n':
                    // 인자가 필요한 옵션: 현재 옵션 문자열의 마지막이고 다음 인자가 있어야 함
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        opts->name_pattern = strdup(argv[++i]);  // 다음 인자를 패턴으로 사용
                        goto next_arg;  // 현재 옵션 문자열 처리 완료
                    } else {
                        fprintf(stderr, "오류: -n 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 'i':
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        opts->iname_pattern = strdup(argv[++i]);  // 대소문자 무시 패턴
                        goto next_arg;
                    } else {
                        fprintf(stderr, "오류: -i 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 's':
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        opts->size_spec = strdup(argv[++i]);  // 크기 조건
                        goto next_arg;
                    } else {
                        fprintf(stderr, "오류: -s 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 'u':
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        opts->user_name = strdup(argv[++i]);  // 사용자명
                        goto next_arg;
                    } else {
                        fprintf(stderr, "오류: -u 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 'p':
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        opts->perm_spec = strdup(argv[++i]);  // 권한 조건
                        goto next_arg;
                    } else {
                        fprintf(stderr, "오류: -p 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 't':
                    if (j == strlen(arg) - 1 && i + 1 < argc) {
                        char *time_arg = argv[++i];
                        
                        // +/- 접두어 확인
                        if (time_arg[0] == '+') {
                            opts->mtime_prefix = '+';
                            opts->mtime_days = atoi(time_arg + 1);
                        } else if (time_arg[0] == '-') {
                            opts->mtime_prefix = '-';
                            opts->mtime_days = atoi(time_arg + 1);
                        } else {
                            opts->mtime_prefix = 0;  // 접두어 없음
                            opts->mtime_days = atoi(time_arg);
                        }
                        
                        opts->mtime_set = true;  // mtime 조건 설정됨 표시
                        goto next_arg;
                    } else {
                        fprintf(stderr, "오류: -t 옵션은 분리해서 사용해야 합니다.\n");
                        return -1;
                    }
                    break;
                    
                case 'h':
                    print_usage(argv[0]);  // 도움말 출력
                    return 1;              // 도움말 출력 후 정상 종료를 의미
                    
                default:
                    fprintf(stderr, "알 수 없는 옵션: -%c\n", arg[j]);
                    return -1;  // 알 수 없는 옵션 오류
            }
        }
        
        // goto 레이블: 인자가 있는 옵션 처리 후 다음 argv로 이동
        next_arg:;
    }
    
    return 0;  // 파싱 성공
}