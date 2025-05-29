#ifndef LS_OPTIONS_H
#define LS_OPTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

/**
 * ls 명령어의 다양한 옵션 플래그들을 저장하는 구조체
 * 각 필드는 해당 옵션이 활성화되었는지를 나타내는 불린 값 또는 관련 데이터를 저장
 */
typedef struct {
    int show_all;           // -a: 숨김 파일(도트로 시작하는 파일) 포함하여 표시
    int long_format;        // -l: 권한, 소유자, 크기, 날짜 등 상세 정보 표시
    int human_readable;     // -h: 파일 크기를 K, M, G 단위로 사람이 읽기 쉽게 표시
    int sort_by_time;       // -t: 파일을 이름 대신 수정 시간 기준으로 정렬
    int show_size;          // -s: 각 파일의 블록 단위 용량을 함께 표시
    int reverse_sort;       // -r: 정렬 순서를 역순으로 변경
    int recursive;          // -R: 하위 디렉토리까지 재귀적으로 모두 표시
    int show_inode;         // -i: 각 파일의 inode 번호를 함께 출력
    int group_by_ext;       // -e: 파일들을 확장자별로 그룹화하여 표시
    int dirs_first;         // -d: 디렉토리를 일반 파일보다 먼저 표시
    char *filter_ext;       // -f [확장자]: 지정된 확장자를 가진 파일만 필터링하여 표시
} ls_options_t;

/**
 * 개별 파일의 정보를 저장하는 구조체
 * 파일명, 통계 정보, 전체 경로를 포함
 */
typedef struct {
    char *name;             // 파일명 (디렉토리 경로 제외)
    struct stat stat_info;  // 파일의 상세 통계 정보 (크기, 권한, 시간 등)
    char *path;             // 파일의 전체 경로
} file_info_t;

// === 옵션 파싱 관련 함수 ===
/**
 * 명령행 인자를 파싱하여 ls_options_t 구조체에 옵션 정보를 저장
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @param options 파싱된 옵션을 저장할 구조체 포인터
 */
void parse_options(int argc, char *argv[], ls_options_t *options);

/**
 * 프로그램 사용법을 출력하는 함수
 * @param program_name 실행 파일명
 */
void print_usage(const char *program_name);

// === 디렉토리 처리 관련 함수 ===
/**
 * 지정된 디렉토리의 내용을 나열하는 메인 함수
 * @param path 나열할 디렉토리 경로
 * @param options 적용할 옵션들
 * @param is_recursive 재귀 호출 여부 (디렉토리명 출력 제어용)
 */
void list_directory(const char *path, ls_options_t *options, int is_recursive);

// === 파일 비교 및 정렬 관련 함수 ===
/**
 * qsort에서 사용할 파일 비교 함수
 * 옵션에 따라 이름, 시간, 확장자, 디렉토리 우선 등 다양한 기준으로 비교
 * @param a 비교할 첫 번째 파일 정보
 * @param b 비교할 두 번째 파일 정보
 * @param options 정렬 기준을 결정하는 옵션들
 * @return 비교 결과 (-1: a < b, 0: a == b, 1: a > b)
 */
int compare_files(const void *a, const void *b, ls_options_t *options);

// === 파일 정보 출력 관련 함수 ===
/**
 * 단일 파일의 정보를 설정된 옵션에 따라 출력
 * @param file 출력할 파일 정보
 * @param options 출력 형식을 결정하는 옵션들
 */
void print_file_info(file_info_t *file, ls_options_t *options);

/**
 * 파일 크기를 옵션에 따라 포맷팅하여 문자열로 변환
 * @param size 원본 파일 크기 (바이트)
 * @param buffer 포맷된 크기를 저장할 버퍼
 * @param human_readable 사람이 읽기 쉬운 형식 사용 여부
 */
void format_size(off_t size, char *buffer, int human_readable);

/**
 * 파일 권한을 문자열로 변환 (예: -rwxr-xr-x)
 * @param mode 파일의 mode 비트
 * @param buffer 권한 문자열을 저장할 버퍼 (최소 11바이트)
 */
void format_permissions(mode_t mode, char *buffer);

// === 유틸리티 함수 ===
/**
 * 파일명에서 확장자를 추출하는 함수
 * @param filename 파일명
 * @return 확장자 포인터 (확장자가 없으면 NULL)
 */
char *get_file_extension(const char *filename);

/**
 * 설정된 옵션에 따라 해당 파일을 표시할지 결정
 * 숨김 파일 옵션, 확장자 필터 등을 고려
 * @param filename 검사할 파일명
 * @param options 필터링 기준이 되는 옵션들
 * @return 표시 여부 (1: 표시, 0: 숨김)
 */
int should_show_file(const char *filename, ls_options_t *options);

/**
 * 파일 정보 배열의 메모리를 해제하는 함수
 * 각 파일의 name, path 메모리와 배열 자체를 모두 해제
 * @param files 해제할 파일 정보 배열
 * @param count 배열의 요소 개수
 */
void free_file_info_array(file_info_t *files, int count);

#endif // LS_OPTIONS_H