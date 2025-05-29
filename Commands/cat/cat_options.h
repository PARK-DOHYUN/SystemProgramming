/**
 * cat_options.h - cat 명령어 구현을 위한 헤더 파일
 * 
 * 이 헤더 파일은 cat 명령어의 옵션 처리와 관련된
 * 구조체, 함수 선언, 필요한 표준 라이브러리를 포함합니다.
 */

#ifndef CAT_OPTIONS_H
#define CAT_OPTIONS_H

// 표준 라이브러리 포함
#include <stdio.h>      // 파일 입출력 함수 (fopen, fclose, printf, fprintf 등)
#include <stdlib.h>     // 일반 유틸리티 함수 (atoi, free 등)
#include <string.h>     // 문자열 처리 함수 (strcmp, strlen 등)
#include <unistd.h>     // POSIX 운영체제 API (getline 등)

/**
 * cat_options_t - cat 명령어의 모든 옵션을 저장하는 구조체
 * 
 * 각 멤버는 해당 옵션의 활성화 여부를 나타내며,
 * 0은 비활성화, 1(또는 양수)는 활성화를 의미합니다.
 */
typedef struct {
    int number_all;        // -n 옵션: 모든 출력 줄에 번호를 매김 (빈 줄 포함)
    int number_nonblank;   // -b 옵션: 빈 줄이 아닌 출력 줄에만 번호를 매김
    int squeeze_blank;     // -s 옵션: 연속된 빈 줄을 하나의 빈 줄로 압축
    int head_lines;        // -h N 옵션: 처음 N줄만 출력 (0이면 모든 줄 출력)
} cat_options_t;

// 함수 선언부

/**
 * init_options - 옵션 구조체를 기본값으로 초기화
 * @opts: 초기화할 cat_options_t 구조체 포인터
 * 
 * 모든 옵션을 비활성화 상태로 설정합니다.
 * 프로그램 시작 시 반드시 호출되어야 합니다.
 */
void init_options(cat_options_t *opts);

/**
 * parse_options - 명령행 인자를 파싱하여 옵션을 설정
 * @argc: main 함수의 argc (명령행 인자 개수)
 * @argv: main 함수의 argv (명령행 인자 배열)
 * @opts: 파싱된 옵션을 저장할 구조체 포인터
 * @file_start_idx: 첫 번째 파일 인자의 인덱스를 저장할 정수 포인터
 * 
 * 지원하는 옵션:
 * - -b: 빈 줄이 아닌 줄에만 번호 매기기
 * - -n: 모든 줄에 번호 매기기
 * - -s: 연속된 빈 줄 압축
 * - -h N: 처음 N줄만 출력
 * - --help: 도움말 출력
 * 
 * @return: 성공 시 0, --help 출력 시 -1, 오류 시 양수
 */
int parse_options(int argc, char *argv[], cat_options_t *opts, int *file_start_idx);

/**
 * print_usage - 프로그램 사용법을 표준 출력으로 출력
 * @program_name: 프로그램 이름 (보통 argv[0])
 * 
 * --help 옵션이나 잘못된 옵션 사용 시 호출되어
 * 사용 가능한 옵션들과 그 기능을 안내합니다.
 */
void print_usage(const char *program_name);

/**
 * is_blank_line - 주어진 문자열이 빈 줄인지 판단
 * @line: 검사할 문자열 (보통 파일에서 읽은 한 줄)
 * 
 * 빈 줄의 정의:
 * - 아무 문자도 없는 줄 (빈 문자열)
 * - 공백문자(space, tab)와 개행문자(LF, CR)만 포함하는 줄
 * 
 * @return: 빈 줄이면 1, 내용이 있는 줄이면 0
 */
int is_blank_line(const char *line);

#endif /* CAT_OPTIONS_H */