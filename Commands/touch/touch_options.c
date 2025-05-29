#include "touch_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * 옵션 구조체 초기화 함수
 * touch_options 구조체의 모든 멤버를 기본값으로 설정한다.
 * 
 * @param opts 초기화할 옵션 구조체 포인터
 */
void init_options(touch_options *opts) {
    opts->access_time = 0;      // 접근 시간 변경 플래그 (기본값: 비활성)
    opts->modify_time = 0;      // 수정 시간 변경 플래그 (기본값: 비활성)
    opts->no_create = 0;        // 파일 생성 금지 플래그 (기본값: 생성 허용)
    opts->create_path = 0;      // 디렉토리 생성 플래그 (기본값: 비활성)
    opts->use_custom_time = 0;  // 사용자 지정 시간 사용 플래그 (기본값: 현재 시간)
    opts->custom_time.tv_sec = 0;   // 사용자 지정 시간 (초)
    opts->custom_time.tv_nsec = 0;  // 사용자 지정 시간 (나노초)
}

/**
 * 명령행 인수 파싱 함수
 * argc와 argv를 분석하여 옵션과 파일 목록을 추출한다.
 * 
 * @param argc 명령행 인수 개수
 * @param argv 명령행 인수 배열
 * @param opts 파싱된 옵션을 저장할 구조체
 * @param files 파일 목록을 저장할 포인터의 포인터
 * @param file_count 파일 개수를 저장할 변수의 포인터
 * @return 성공 시 0, 실패 시 -1
 */
int parse_options(int argc, char *argv[], touch_options *opts, char ***files, int *file_count) {
    static char **file_list = NULL;  // 파일 목록 저장용 정적 배열
    int files_allocated = 0;         // 할당된 파일 목록 크기
    *file_count = 0;                 // 파일 개수 초기화
    
    // 모든 명령행 인수를 순회
    for (int i = 1; i < argc; i++) {
        // 옵션인지 확인 ('-'로 시작하고 빈 문자열이 아님)
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            // -t 옵션 특별 처리 (시간 지정 옵션)
            if (strncmp(argv[i], "-t", 2) == 0) {
                char *time_str = NULL;
                if (argv[i][2] != '\0') {
                    // "-t202312251430" 형태 (붙여서 작성)
                    time_str = &argv[i][2];
                } else if (i + 1 < argc) {
                    // "-t 202312251430" 형태 (공백으로 분리)
                    time_str = argv[++i];
                } else {
                    fprintf(stderr, "옵션 -t에는 시간 인수가 필요합니다\n");
                    return -1;
                }
                
                // 시간 문자열을 파싱하여 timespec 구조체로 변환
                if (parse_time_string(time_str, &opts->custom_time) != 0) {
                    fprintf(stderr, "잘못된 시간 형식: %s\n", time_str);
                    return -1;
                }
                opts->use_custom_time = 1;
                continue;
            }
            
            // 다른 옵션들 처리 (묶음 옵션 지원: -amp 같은 형태)
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        opts->access_time = 1;  // 접근 시간만 변경
                        break;
                    case 'm':
                        opts->modify_time = 1;  // 수정 시간만 변경
                        break;
                    case 'c':
                        opts->no_create = 1;    // 파일이 없어도 생성하지 않음
                        break;
                    case 'p':
                        opts->create_path = 1;  // 필요시 중간 디렉토리 생성
                        break;
                    default:
                        fprintf(stderr, "알 수 없는 옵션: -%c\n", argv[i][j]);
                        return -1;
                }
            }
        } else {
            // 파일명으로 인식하여 목록에 추가
            // 배열 크기가 부족하면 동적으로 확장
            if (*file_count >= files_allocated) {
                files_allocated = files_allocated ? files_allocated * 2 : 8;
                file_list = realloc(file_list, files_allocated * sizeof(char*));
                if (!file_list) {
                    perror("메모리 할당 실패");
                    return -1;
                }
            }
            file_list[*file_count] = argv[i];
            (*file_count)++;
        }
    }
    
    *files = file_list;
    
    // 기본값 처리: -a나 -m 옵션이 없으면 둘 다 활성화
    // (일반적인 touch 동작은 접근 시간과 수정 시간을 모두 변경)
    if (!opts->access_time && !opts->modify_time) {
        opts->access_time = 1;
        opts->modify_time = 1;
    }
    
    return 0;
}

/**
 * 시간 문자열 파싱 함수
 * -t 옵션에서 사용되는 시간 문자열을 파싱하여 timespec 구조체로 변환한다.
 * 지원 형식: [[CC]YY]MMDDhhmm[.ss]
 * 
 * @param time_str 파싱할 시간 문자열
 * @param ts 결과를 저장할 timespec 구조체
 * @return 성공 시 0, 실패 시 -1
 */
int parse_time_string(const char *time_str, struct timespec *ts) {
    struct tm tm_time = {0};     // 시간 구조체 초기화
    time_t now = time(NULL);     // 현재 시간 획득
    struct tm *current = localtime(&now);  // 현재 로컬 시간
    
    // 현재 시간을 기본값으로 설정 (지정되지 않은 필드 처리용)
    tm_time = *current;
    
    int len = strlen(time_str);
    char temp_str[32];
    // 임시 문자열로 복사 (원본 수정 방지)
    strncpy(temp_str, time_str, sizeof(temp_str) - 1);
    temp_str[sizeof(temp_str) - 1] = '\0';
    
    // 초 부분 분리 (.ss 형태)
    char *dot = strchr(temp_str, '.');
    int seconds = 0;
    if (dot) {
        *dot = '\0';  // 점을 null로 바꿔서 문자열 분리
        seconds = atoi(dot + 1);  // 점 다음 부분을 초로 변환
        if (seconds < 0 || seconds > 59) {
            return -1;  // 초 범위 검증
        }
        len = strlen(temp_str);  // 점 이전 부분의 길이로 업데이트
    }
    
    // 문자열 길이에 따른 시간 형식 파싱
    switch (len) {
        case 8: // MMDDhhmm (월일시분)
            if (sscanf(temp_str, "%2d%2d%2d%2d", 
                      &tm_time.tm_mon, &tm_time.tm_mday, 
                      &tm_time.tm_hour, &tm_time.tm_min) != 4) {
                return -1;
            }
            tm_time.tm_mon -= 1; // tm_mon은 0~11 (1월=0)
            break;
            
        case 10: // YYMMDDhhmm (년월일시분, 2자리 년도)
            if (sscanf(temp_str, "%2d%2d%2d%2d%2d", 
                      &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
                      &tm_time.tm_hour, &tm_time.tm_min) != 5) {
                return -1;
            }
            // 2자리 년도를 4자리로 변환 (70 이상이면 19xx, 미만이면 20xx)
            tm_time.tm_year += (tm_time.tm_year >= 70) ? 0 : 100;
            tm_time.tm_mon -= 1;
            break;
            
        case 12: // CCYYMMDDhhmm (년월일시분, 4자리 년도)
            if (sscanf(temp_str, "%4d%2d%2d%2d%2d", 
                      &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
                      &tm_time.tm_hour, &tm_time.tm_min) != 5) {
                return -1;
            }
            tm_time.tm_year -= 1900; // tm_year는 1900년부터의 년수
            tm_time.tm_mon -= 1;
            break;
            
        default:
            return -1;  // 지원하지 않는 형식
    }
    
    tm_time.tm_sec = seconds;
    
    // 입력값 유효성 검사
    if (tm_time.tm_mon < 0 || tm_time.tm_mon > 11 ||      // 월: 0~11
        tm_time.tm_mday < 1 || tm_time.tm_mday > 31 ||    // 일: 1~31
        tm_time.tm_hour < 0 || tm_time.tm_hour > 23 ||    // 시: 0~23
        tm_time.tm_min < 0 || tm_time.tm_min > 59) {      // 분: 0~59
        return -1;
    }
    
    // tm 구조체를 time_t로 변환
    ts->tv_sec = mktime(&tm_time);
    ts->tv_nsec = 0;  // 나노초는 0으로 설정
    
    return (ts->tv_sec == -1) ? -1 : 0;  // mktime 실패 검사
}

/**
 * 사용법 출력 함수
 * 프로그램의 사용법과 옵션 설명을 출力한다.
 * 
 * @param program_name 프로그램 이름 (argv[0])
 */
void print_usage(const char *program_name) {
    printf("사용법: %s [옵션]... 파일...\n", program_name);
    printf("옵션:\n");
    printf("  -a          접근 시간만 변경\n");
    printf("  -m          수정 시간만 변경\n");
    printf("  -c          파일이 존재하지 않으면 생성하지 않음\n");
    printf("  -p          필요한 경우 중간 디렉토리 생성\n");
    printf("  -t 시간     지정된 시간으로 설정 ([[CC]YY]MMDDhhmm[.ss])\n");
    printf("\n예시:\n");
    printf("  %s file.txt\n", program_name);                    // 기본 사용
    printf("  %s -cmp file.txt\n", program_name);               // 옵션 조합
    printf("  %s -t 202312251430.30 file.txt\n", program_name); // 시간 지정
}