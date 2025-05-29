#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include "mv_options.h"

/**
 * 주어진 경로가 디렉토리인지 확인하는 함수
 * @param path 검사할 경로 문자열
 * @return 디렉토리이면 true, 아니면 false
 * 
 * stat() 시스템 콜을 사용하여 파일 정보를 가져오고,
 * S_ISDIR 매크로로 디렉토리 여부를 판단한다.
 */
bool is_directory(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

/**
 * 실제 파일 이동/이름변경을 수행하는 핵심 함수
 * @param src 원본 파일 경로
 * @param dest 대상 파일/디렉토리 경로
 * @param opts 명령줄 옵션 구조체 포인터
 * @return 성공시 0, 실패시 -1
 * 
 * 이 함수는 다음 작업들을 순차적으로 수행한다:
 * 1. 대상이 디렉토리인 경우 파일명을 추가하여 최종 경로 생성
 * 2. -s 옵션 처리: 중복 파일명 시 고유한 이름 생성
 * 3. 덮어쓰기 여부 확인 (옵션에 따라)
 * 4. rename() 시스템 콜로 실제 이동 수행
 * 5. -v 옵션 시 상세 정보 출력
 */
int perform_move(const char *src, const char *dest, mv_options_t *opts) {
    char *final_dest = NULL;        // 최종 대상 경로
    char *unique_dest = NULL;       // -s 옵션으로 생성된 고유 경로
    bool allocated_dest = false;    // 메모리 할당 여부 추적 플래그
    int result = 0;                 // 함수 반환값
    
    // 대상이 디렉토리인 경우, 원본 파일명을 유지하여 디렉토리 내부로 이동
    // 예: mv file.txt /home/user/ -> /home/user/file.txt
    if (is_directory(dest)) {
        char *src_basename = basename((char*)src);  // 원본 파일의 기본 이름 추출
        // 디렉토리 경로 + '/' + 파일명 + null terminator를 위한 메모리 할당
        final_dest = malloc(strlen(dest) + strlen(src_basename) + 2);
        snprintf(final_dest, strlen(dest) + strlen(src_basename) + 2, "%s/%s", dest, src_basename);
        allocated_dest = true;  // 메모리를 할당했음을 표시
    } else {
        // 대상이 파일인 경우 그대로 사용
        final_dest = (char*)dest;
    }
    
    // -s 옵션 처리: 중복 파일이 있을 때 고유한 이름 생성 (예: file_1.txt)
    if (opts->suffix) {
        unique_dest = generate_unique_name(final_dest);
        if (allocated_dest) {
            free(final_dest);  // 이전에 할당한 메모리 해제
        }
        final_dest = unique_dest;
        allocated_dest = true;
    } else {
        // -s 옵션이 없는 경우 덮어쓰기 여부 확인
        if (!should_overwrite(final_dest, opts)) {
            if (opts->verbose) {
                printf("'%s' 이동이 취소되었습니다.\n", src);
            }
            // 메모리 정리 후 함수 종료
            if (allocated_dest) {
                free(final_dest);
            }
            return 0;
        }
    }
    
    // rename() 시스템 콜을 사용하여 실제 파일 이동 수행
    // 같은 파일시스템 내에서는 빠른 이동, 다른 파일시스템 간에는 복사 후 삭제
    if (rename(src, final_dest) != 0) {
        fprintf(stderr, "mv: '%s'에서 '%s'로 이동할 수 없습니다: %s\n", 
                src, final_dest, strerror(errno));
        result = -1;
    } else {
        // -v 옵션: 이동 성공 시 상세 정보 출력
        if (opts->verbose) {
            printf("'%s' -> '%s'\n", src, final_dest);
        }
    }
    
    // 할당된 메모리가 있다면 해제하여 메모리 누수 방지
    if (allocated_dest) {
        free(final_dest);
    }
    
    return result;
}

/**
 * 프로그램의 메인 함수
 * 명령줄 인자를 파싱하고 각 파일에 대해 이동 작업을 수행한다.
 * 
 * 프로그램 실행 흐름:
 * 1. 옵션 초기화 및 파싱
 * 2. 인자 개수 유효성 검사
 * 3. 원본 파일들과 대상 경로 분리
 * 4. 여러 파일 이동 시 대상이 디렉토리인지 확인
 * 5. 각 원본 파일에 대해 이동 작업 수행
 */
int main(int argc, char *argv[]) {
    mv_options_t opts;              // 명령줄 옵션을 저장할 구조체
    int first_file_index;           // 옵션이 아닌 첫 번째 인자의 인덱스
    struct stat st;                 // 파일 상태 정보를 위한 구조체
    
    // 옵션 구조체의 모든 필드를 기본값(false)으로 초기화
    init_options(&opts);
    
    // 명령줄에서 옵션들(-i, -f, -n, -v, -s)을 파싱
    // 반환값은 옵션이 아닌 첫 번째 인자의 인덱스
    first_file_index = parse_options(argc, argv, &opts);
    if (first_file_index == -1) {
        return 1;  // 옵션 파싱 실패
    }
    
    // 최소한 원본과 대상 2개의 인자가 필요
    if (argc - first_file_index < 2) {
        fprintf(stderr, "사용법: mv [-ifnvs] 원본 대상\n");
        fprintf(stderr, "       mv [-ifnvs] 원본1 원본2 ... 대상디렉토리\n");
        return 1;
    }
    
    // 명령줄 인자를 원본 파일들과 대상 경로로 분리
    // 마지막 인자는 항상 대상, 나머지는 원본 파일들
    int num_sources = argc - first_file_index - 1;     // 원본 파일 개수
    char **sources = &argv[first_file_index];          // 원본 파일들의 배열 포인터
    char *destination = argv[argc - 1];                // 대상 경로 (마지막 인자)
    
    // 여러 파일을 이동하는 경우, 대상은 반드시 기존 디렉토리여야 함
    // 예: mv file1 file2 file3 /home/user/ (O)
    //     mv file1 file2 file3 newfile (X - newfile이 디렉토리가 아니면 오류)
    if (num_sources > 1 && !is_directory(destination)) {
        fprintf(stderr, "mv: 대상 '%s'가 디렉토리가 아닙니다\n", destination);
        return 1;
    }
    
    // 각 원본 파일에 대해 순차적으로 이동 작업 수행
    for (int i = 0; i < num_sources; i++) {
        // stat() 시스템 콜로 원본 파일의 존재 여부 및 접근 권한 확인
        if (stat(sources[i], &st) != 0) {
            fprintf(stderr, "mv: '%s'에 접근할 수 없습니다: %s\n", 
                    sources[i], strerror(errno));
            continue;  // 현재 파일은 건너뛰고 다음 파일 처리
        }
        
        // 원본과 대상이 동일한 파일인지 확인 (자기 자신으로 이동 방지)
        // 예: mv file.txt file.txt
        if (strcmp(sources[i], destination) == 0) {
            if (opts.verbose) {
                printf("'%s'와 '%s'는 같은 파일입니다\n", sources[i], destination);
            }
            continue;  // 같은 파일이면 건너뛰기
        }
        
        // 실제 파일 이동 수행
        // 하나라도 실패하면 프로그램 전체가 실패로 종료
        if (perform_move(sources[i], destination, &opts) != 0) {
            return 1;
        }
    }
    
    return 0;  // 모든 파일 이동 성공
}