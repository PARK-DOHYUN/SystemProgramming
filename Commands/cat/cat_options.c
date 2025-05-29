#include "cat_options.h"

/**
 * init_options - cat_options_t 구조체를 기본값으로 초기화
 * @opts: 초기화할 옵션 구조체 포인터
 * 
 * 모든 옵션을 비활성화 상태(0)로 설정
 * 프로그램 시작 시 반드시 호출되어야 함
 */
void init_options(cat_options_t *opts) {
    opts->number_all = 0;      // -n 옵션: 모든 줄 번호 매기기 비활성화
    opts->number_nonblank = 0; // -b 옵션: 빈 줄 제외 번호 매기기 비활성화
    opts->squeeze_blank = 0;   // -s 옵션: 빈 줄 압축 비활성화
    opts->head_lines = 0;      // -h 옵션: 줄 수 제한 없음 (0 = 모든 줄 출력)
}

/**
 * print_usage - 프로그램 사용법을 표준 출력으로 출력
 * @program_name: 프로그램 이름 (argv[0])
 * 
 * --help 옵션이나 잘못된 옵션 사용 시 호출됨
 * 각 옵션의 기능과 사용법을 설명
 */
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTION]... [FILE]...\n", program_name);
    printf("Concatenate FILE(s) to standard output.\n\n");
    printf("Options:\n");
    printf("  -b          number nonempty output lines\n");      // 빈 줄 제외하고 줄 번호
    printf("  -n          number all output lines\n");           // 모든 줄에 줄 번호
    printf("  -s          suppress repeated empty output lines\n"); // 연속 빈 줄 압축
    printf("  -h N        output only first N lines\n");         // 처음 N줄만 출력
    printf("  --help      display this help and exit\n");        // 도움말 출력
}

/**
 * is_blank_line - 주어진 문자열이 빈 줄인지 확인
 * @line: 검사할 문자열
 * 
 * 빈 줄의 정의: 공백문자(space, tab), 개행문자(CR, LF)만 포함하거나 빈 문자열
 * 
 * @return: 빈 줄이면 1, 아니면 0
 */
int is_blank_line(const char *line) {
    // 문자열의 각 문자를 검사
    while (*line) {
        // 공백, 탭, 개행, 캐리지 리턴이 아닌 문자가 있으면 빈 줄이 아님
        if (*line != ' ' && *line != '\t' && *line != '\n' && *line != '\r') {
            return 0;
        }
        line++;
    }
    return 1; // 모든 문자가 공백문자이거나 빈 문자열
}

/**
 * parse_options - 명령행 인자를 파싱하여 옵션 설정
 * @argc: 명령행 인자 개수
 * @argv: 명령행 인자 배열
 * @opts: 파싱 결과를 저장할 옵션 구조체 포인터
 * @file_start_idx: 첫 번째 파일 인자의 인덱스를 저장할 포인터
 * 
 * 지원하는 옵션:
 * - 단일 옵션: -b, -n, -s, -h N, --help
 * - 묶음 옵션: -bns, -h5 등
 * - 특수 처리: -b와 -n이 동시 지정 시 -b 우선
 * 
 * @return: 성공 시 0, --help 시 -1, 오류 시 양수
 */
int parse_options(int argc, char *argv[], cat_options_t *opts, int *file_start_idx) {
    int i;
    
    // 기본적으로 첫 번째 인자부터 파일로 가정
    *file_start_idx = 1;
    
    // 명령행 인자를 하나씩 검사
    for (i = 1; i < argc; i++) {
        // '-'로 시작하지 않으면 파일명으로 간주하고 파싱 종료
        if (argv[i][0] != '-') {
            *file_start_idx = i;
            break;
        }
        
        // --help 옵션 처리
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return -1; // 도움말 출력 후 프로그램 정상 종료 신호
        }
        
        // -h 옵션 단독 사용 처리 (다음 인자가 숫자여야 함)
        if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                // 다음 인자를 숫자로 변환
                opts->head_lines = atoi(argv[i + 1]);
                if (opts->head_lines <= 0) {
                    fprintf(stderr, "Error: Invalid number for -h option\n");
                    return 1;
                }
                i++; // 숫자 인자를 건너뛰기 위해 증가
                continue;
            } else {
                fprintf(stderr, "Error: -h option requires a number\n");
                return 1;
            }
        }
        
        // 묶음 옵션 처리 (-bns, -h5 등)
        char *opt_ptr = argv[i] + 1; // '-' 다음 문자부터 시작
        while (*opt_ptr) {
            switch (*opt_ptr) {
                case 'b':
                    opts->number_nonblank = 1; // 빈 줄 제외 줄 번호 활성화
                    break;
                case 'n':
                    opts->number_all = 1;      // 모든 줄 번호 활성화
                    break;
                case 's':
                    opts->squeeze_blank = 1;   // 빈 줄 압축 활성화
                    break;
                case 'h':
                    // -h가 묶음 옵션 내에서 사용될 때 (예: -h5)
                    opt_ptr++; // 'h' 다음 문자로 이동
                    if (*opt_ptr) {
                        // 남은 문자들을 숫자로 해석
                        opts->head_lines = atoi(opt_ptr);
                        if (opts->head_lines <= 0) {
                            fprintf(stderr, "Error: Invalid number for -h option\n");
                            return 1;
                        }
                        goto next_arg; // 나머지 문자들을 숫자로 처리했으므로 다음 인자로
                    } else {
                        fprintf(stderr, "Error: -h option requires a number\n");
                        return 1;
                    }
                    break;
                default:
                    // 알 수 없는 옵션
                    fprintf(stderr, "Error: Unknown option -%c\n", *opt_ptr);
                    return 1;
            }
            opt_ptr++; // 다음 옵션 문자로 이동
        }
        next_arg:; // goto문의 목적지 (묶음 옵션에서 -h 처리 후 사용)
    }
    
    // 옵션 충돌 해결: -b와 -n이 동시에 지정된 경우 -b가 우선
    // (GNU cat의 동작과 동일)
    if (opts->number_nonblank && opts->number_all) {
        opts->number_all = 0;
    }
    
    // 파일 시작 인덱스 설정
    *file_start_idx = i;
    return 0; // 성공
}