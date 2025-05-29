/**
 * touch_options.h
 * 
 * touch 프로그램의 옵션 관리를 위한 헤더 파일
 * 옵션 구조체 정의와 관련 함수들의 프로토타입을 포함한다.
 */

#ifndef TOUCH_OPTIONS_H
#define TOUCH_OPTIONS_H

#include <time.h>

/**
 * touch 프로그램의 모든 옵션을 저장하는 구조체
 * 각 멤버는 해당 옵션의 활성화 여부나 설정값을 나타낸다.
 */
typedef struct {
    int access_time;     // -a 옵션: 접근 시간만 변경할지 여부 (1=활성, 0=비활성)
    int modify_time;     // -m 옵션: 수정 시간만 변경할지 여부 (1=활성, 0=비활성)
    int no_create;       // -c 옵션: 파일이 없을 때 생성하지 않을지 여부 (1=생성안함, 0=생성함)
    int create_path;     // -p 옵션: 중간 디렉토리를 생성할지 여부 (1=생성, 0=생성안함)
    int use_custom_time; // -t 옵션: 사용자 지정 시간을 사용할지 여부 (1=사용, 0=현재시간)
    struct timespec custom_time; // -t 옵션으로 지정된 사용자 정의 시간 (초와 나노초)
} touch_options;

/**
 * 옵션 구조체 초기화 함수
 * touch_options 구조체의 모든 멤버를 기본값으로 설정한다.
 * 
 * @param opts 초기화할 옵션 구조체의 포인터
 */
void init_options(touch_options *opts);

/**
 * 명령행 인수 파싱 함수
 * main()에서 받은 argc, argv를 분석하여 옵션과 파일 목록을 분리한다.
 * 지원하는 옵션: -a, -m, -c, -p, -t
 * 옵션들은 묶어서 사용 가능 (예: -amp)
 * 
 * @param argc 명령행 인수 개수 (main의 argc)
 * @param argv 명령행 인수 배열 (main의 argv)
 * @param opts 파싱된 옵션들을 저장할 구조체
 * @param files 파일 목록을 저장할 포인터의 포인터 (동적 할당됨)
 * @param file_count 파일 개수를 저장할 변수의 포인터
 * @return 성공 시 0, 파싱 오류 시 -1
 */
int parse_options(int argc, char *argv[], touch_options *opts, char ***files, int *file_count);

/**
 * 시간 문자열 파싱 함수 (-t 옵션용)
 * -t 옵션에서 사용되는 시간 문자열을 파싱하여 timespec 구조체로 변환한다.
 * 
 * 지원하는 시간 형식:
 * - MMDDhhmm (8자리): 월일시분
 * - YYMMDDhhmm (10자리): 2자리년도 + 월일시분  
 * - CCYYMMDDhhmm (12자리): 4자리년도 + 월일시분
 * - 위 형식 뒤에 .ss 추가 가능 (초 지정)
 * 
 * 예시: 202312251430.30 = 2023년 12월 25일 14시 30분 30초
 * 
 * @param time_str 파싱할 시간 문자열
 * @param ts 파싱 결과를 저장할 timespec 구조체
 * @return 성공 시 0, 형식 오류 시 -1
 */
int parse_time_string(const char *time_str, struct timespec *ts);

/**
 * 사용법 출력 함수
 * 프로그램의 올바른 사용법과 각 옵션의 설명을 표준 출력으로 출력한다.
 * 잘못된 인수가 주어졌거나 도움이 필요할 때 호출된다.
 * 
 * @param program_name 프로그램 이름 (보통 argv[0])
 */
void print_usage(const char *program_name);

#endif /* TOUCH_OPTIONS_H */