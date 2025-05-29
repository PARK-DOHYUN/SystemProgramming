/*
 * find_options.h - find 명령어 옵션 구조체 및 함수 선언
 * 
 * 이 헤더 파일은 find 프로그램에서 사용하는 검색 옵션들을 관리하기 위한
 * 구조체와 관련 함수들을 정의합니다.
 * 
 * 주요 구성요소:
 * - find_options_t: 모든 검색 조건을 담는 구조체
 * - 옵션 파싱 및 관리 함수들
 */

#ifndef FIND_OPTIONS_H
#define FIND_OPTIONS_H

#include <stdbool.h>
#include <sys/types.h>

/**
 * find 명령어의 모든 검색 옵션을 담는 구조체
 * 
 * 이 구조체는 사용자가 지정한 모든 검색 조건들을 저장합니다.
 * 각 필드는 특정 검색 조건에 해당하며, 설정되지 않은 조건은
 * 기본값(false, NULL, -1 등)을 가집니다.
 */
typedef struct {
    // === 파일 타입 필터 ===
    bool type_file;         /**< -f 옵션: true이면 일반 파일만 검색 */
    bool type_dir;          /**< -d 옵션: true이면 디렉토리만 검색 */
    bool empty_filter;      /**< -e 옵션: true이면 빈 파일/디렉토리만 검색 */
    
    // === 이름 패턴 매칭 ===
    char *name_pattern;     /**< -n 옵션: 파일명 패턴 (대소문자 구분, shell glob 패턴) */
    char *iname_pattern;    /**< -i 옵션: 파일명 패턴 (대소문자 무시, shell glob 패턴) */
    
    // === 파일 속성 조건 ===
    char *size_spec;        /**< -s 옵션: 파일 크기 조건 (+100, -1k, 5M 등) */
    char *user_name;        /**< -u 옵션: 파일 소유자 사용자명 */
    char *perm_spec;        /**< -p 옵션: 파일 권한 (8진수 문자열, 예: "755") */
    
    // === 시간 조건 ===
    int mtime_days;         /**< -t 옵션: 수정 시간 (현재로부터 n일) */
    char mtime_prefix;      /**< mtime 접두어: '+' (이전), '-' (이내), 0 (정확히) */
    bool mtime_set;         /**< mtime 조건이 설정되었는지 여부 플래그 */
    
    // === 검색 경로 ===
    char *search_path;      /**< 검색을 시작할 기본 경로 (기본값: 현재 디렉토리 ".") */
} find_options_t;

// === 함수 선언 ===

/**
 * 명령행 인자를 파싱하여 옵션 구조체에 설정
 * 
 * 이 함수는 argc, argv로 전달된 명령행 인자들을 분석하여
 * find_options_t 구조체에 적절한 값들을 설정합니다.
 * 
 * 지원하는 옵션 형태:
 * - 개별 옵션: -f -d -e
 * - 묶음 옵션: -fde (여러 플래그를 한번에)
 * - 인자가 있는 옵션: -n pattern, -s +100k
 * - 시간 조건: -t +7 (7일 이전), -t -3 (3일 이내), -t 5 (정확히 5일 전)
 * - 경로 지정: find /path/to/search -f
 * 
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @param opts 파싱 결과를 저장할 옵션 구조체 포인터
 * @return 성공시 0, 도움말 출력시 1, 오류시 -1
 */
int parse_options(int argc, char *argv[], find_options_t *opts);

/**
 * 옵션 구조체를 기본값으로 초기화
 * 
 * 구조체의 모든 필드를 안전한 기본값으로 설정합니다.
 * - 모든 불린 플래그: false
 * - 모든 문자열 포인터: NULL (단, search_path는 "."로 설정)
 * - 숫자 값들: 적절한 기본값
 * 
 * @param opts 초기화할 옵션 구조체 포인터
 */
void init_options(find_options_t *opts);

/**
 * 동적 할당된 메모리 해제
 * 
 * 옵션 구조체 내의 모든 동적 할당된 문자열 메모리를 해제합니다.
 * 프로그램 종료 전에 반드시 호출해야 메모리 누수를 방지할 수 있습니다.
 * 
 * 해제되는 필드들:
 * - name_pattern, iname_pattern
 * - size_spec, user_name, perm_spec
 * - search_path
 * 
 * @param opts 메모리를 해제할 옵션 구조체 포인터
 */
void free_options(find_options_t *opts);

/**
 * 프로그램 사용법을 표준 출력에 출력
 * 
 * 사용자가 -h 옵션을 사용하거나 잘못된 인자를 제공했을 때
 * 올바른 사용법과 옵션 설명을 보여줍니다.
 * 
 * 출력 내용:
 * - 기본 사용법 형식
 * - 각 옵션의 설명
 * - 사용 예시
 * 
 * @param prog_name 프로그램 이름 (보통 argv[0])
 */
void print_usage(const char *prog_name);

#endif /* FIND_OPTIONS_H */