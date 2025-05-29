#ifndef RM_OPTIONS_H
#define RM_OPTIONS_H

#include <stdbool.h>

/**
 * rm 명령어 옵션을 저장하는 구조체
 * 
 * 각 멤버는 해당하는 명령행 옵션의 활성화 여부를 나타냄
 * 모든 멤버는 bool 타입으로 true/false 상태를 가짐
 */
typedef struct {
    bool recursive;     // -r, --recursive: 디렉토리와 그 내용을 재귀적으로 삭제
    bool force;         // -f, --force: 존재하지 않는 파일 무시, 확인 프롬프트 없음
    bool interactive;   // -i, --interactive: 각 삭제 전에 사용자 확인 요청
    bool verbose;       // -v, --verbose: 수행되는 작업에 대한 상세한 설명 출력
    bool zero_only;     // -z, --zero: 0바이트 파일만 삭제 (사용자 정의 옵션)
} rm_options_t;

/**
 * 옵션 구조체 초기화 함수
 * @param opts 초기화할 옵션 구조체 포인터
 * 
 * 모든 옵션을 기본값(false)으로 설정
 * 프로그램 시작시 반드시 호출되어야 함
 */
void init_options(rm_options_t *opts);

/**
 * 명령행 옵션 파싱 함수
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @param opts 파싱 결과를 저장할 옵션 구조체 포인터
 * @param file_start_index 파일 인자가 시작되는 인덱스를 저장할 포인터
 * @return 0: 파싱 성공, -1: 파싱 실패
 * 
 * 단축 옵션(-r), 긴 옵션(--recursive), 묶음 옵션(-rfv) 모두 지원
 * 옵션과 파일 인자를 구분하여 파일 인자 시작 위치를 반환
 */
int parse_options(int argc, char *argv[], rm_options_t *opts, int *file_start_index);

/**
 * 프로그램 사용법 출력 함수
 * @param program_name 프로그램 이름 (보통 argv[0])
 * 
 * 도움말 요청(-h, --help)이나 잘못된 사용시 호출
 * 지원하는 모든 옵션과 사용 예시를 표준 출력으로 출력
 */
void print_usage(const char *program_name);

#endif