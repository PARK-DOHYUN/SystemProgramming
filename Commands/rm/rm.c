#include "rm_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

/**
 * 사용자 확인 입력 받기
 * @param filepath 삭제할 파일의 경로
 * @return true: 사용자가 'y' 또는 'Y' 입력시, false: 그 외의 경우
 * 
 * 대화형 모드(-i 옵션)에서 파일을 삭제하기 전에 사용자에게 확인을 요청하는 함수
 * "rm: remove 'filename'? " 형태로 출력하고 사용자 입력을 기다림
 */
static bool get_user_confirmation(const char *filepath) {
    printf("rm: remove '%s'? ", filepath);
    fflush(stdout);  // 출력 버퍼를 즉시 비워서 프롬프트가 바로 표시되도록 함
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL) {
        return false;  // 입력 오류 시 삭제하지 않음
    }
    
    // 첫 번째 문자가 'y' 또는 'Y'인 경우에만 삭제 승인
    return (response[0] == 'y' || response[0] == 'Y');
}

/**
 * 파일 크기가 0인지 확인
 * @param filepath 검사할 파일의 경로
 * @return true: 0바이트 일반 파일인 경우, false: 그 외의 경우
 * 
 * -z 옵션 사용시 0바이트 파일만 삭제하기 위해 파일 크기를 확인하는 함수
 * stat() 시스템 콜을 사용하여 파일 정보를 얻어 크기와 타입을 확인
 */
static bool is_zero_byte_file(const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return false;  // stat 실패시 0바이트 파일이 아닌 것으로 간주
    }
    // 일반 파일이면서 크기가 0인 경우에만 true 반환
    return (S_ISREG(st.st_mode) && st.st_size == 0);
}

/**
 * 디렉토리인지 확인
 * @param filepath 검사할 경로
 * @return true: 디렉토리인 경우, false: 그 외의 경우
 * 
 * 주어진 경로가 디렉토리인지 확인하여 재귀 삭제 여부를 결정하는데 사용
 */
static bool is_directory(const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return false;  // stat 실패시 디렉토리가 아닌 것으로 간주
    }
    return S_ISDIR(st.st_mode);  // 디렉토리 타입인지 확인
}

/**
 * 경로 결합 함수
 * @param dir 디렉토리 경로
 * @param name 파일/디렉토리 이름
 * @return 결합된 전체 경로 (동적 할당된 메모리, 사용 후 free 필요)
 * 
 * 디렉토리 재귀 삭제시 부모 디렉토리 경로와 자식 요소 이름을 결합하여
 * 전체 경로를 생성하는 함수. 필요시 '/' 구분자를 자동으로 추가함
 */
static char* join_path(const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    // +2는 '/' 구분자와 null terminator를 위한 공간
    char *path = malloc(dir_len + name_len + 2);
    
    if (path == NULL) {
        return NULL;  // 메모리 할당 실패
    }
    
    strcpy(path, dir);
    // 디렉토리 경로 끝에 '/'가 없으면 추가
    if (dir_len > 0 && dir[dir_len - 1] != '/') {
        strcat(path, "/");
    }
    strcat(path, name);
    
    return path;
}

/**
 * 디렉토리 재귀 삭제
 * @param dirpath 삭제할 디렉토리 경로
 * @param opts 삭제 옵션
 * @return 0: 성공, -1: 실패
 * 
 * 디렉토리 내의 모든 파일과 하위 디렉토리를 재귀적으로 삭제한 후
 * 디렉토리 자체를 삭제하는 함수. -r 옵션이 활성화된 경우에 호출됨
 */
static int remove_directory_recursive(const char *dirpath, const rm_options_t *opts) {
    DIR *dir = opendir(dirpath);
    if (dir == NULL) {
        // 디렉토리 열기 실패시 에러 메시지 출력 (force 모드가 아닌 경우)
        if (!opts->force) {
            fprintf(stderr, "rm: cannot open directory '%s': %s\n", 
                    dirpath, strerror(errno));
        }
        return -1;
    }
    
    struct dirent *entry;
    int result = 0;
    
    // 디렉토리 내의 모든 항목을 순회
    while ((entry = readdir(dir)) != NULL) {
        // 현재 디렉토리(.)와 부모 디렉토리(..) 건너뛰기
        // 이를 삭제하려고 하면 무한 루프나 시스템 오류가 발생할 수 있음
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 디렉토리 경로와 항목 이름을 결합하여 전체 경로 생성
        char *filepath = join_path(dirpath, entry->d_name);
        if (filepath == NULL) {
            fprintf(stderr, "rm: memory allocation failed\n");
            result = -1;
            break;
        }
        
        // 각 항목을 재귀적으로 삭제 (파일이면 파일 삭제, 디렉토리면 재귀 호출)
        if (remove_file(filepath, opts) != 0) {
            result = -1;  // 하나라도 실패하면 전체 실패로 간주
        }
        
        free(filepath);  // 동적 할당된 경로 메모리 해제
    }
    
    closedir(dir);
    
    // 디렉토리 내용을 모두 성공적으로 삭제한 경우에만 디렉토리 자체 삭제
    if (result == 0) {
        // 대화형 모드에서 디렉토리 삭제 확인
        if (opts->interactive && !get_user_confirmation(dirpath)) {
            return 0;  // 사용자가 거부하면 삭제하지 않음
        }
        
        // rmdir() 시스템 콜로 빈 디렉토리 삭제
        if (rmdir(dirpath) != 0) {
            if (!opts->force) {
                fprintf(stderr, "rm: cannot remove directory '%s': %s\n", 
                        dirpath, strerror(errno));
            }
            result = -1;
        } else if (opts->verbose) {
            // verbose 모드에서 삭제 완료 메시지 출력
            printf("removed directory '%s'\n", dirpath);
        }
    }
    
    return result;
}

/**
 * 파일 삭제 함수 (메인 삭제 로직)
 * @param filepath 삭제할 파일/디렉토리 경로
 * @param opts 삭제 옵션 구조체
 * @return 0: 성공, -1: 실패
 * 
 * 모든 삭제 옵션을 고려하여 파일이나 디렉토리를 삭제하는 핵심 함수
 * 옵션에 따라 다양한 조건을 확인하고 적절한 삭제 방법을 선택함
 */
int remove_file(const char *filepath, const rm_options_t *opts) {
    // 1단계: 파일/디렉토리 존재 여부 확인
    if (access(filepath, F_OK) != 0) {
        // 파일이 존재하지 않는 경우
        if (!opts->force) {
            // force 모드가 아니면 에러 메시지 출력
            fprintf(stderr, "rm: cannot remove '%s': No such file or directory\n", filepath);
        }
        // force 모드에서는 존재하지 않는 파일을 무시하고 성공으로 처리
        return opts->force ? 0 : -1;
    }
    
    // 2단계: -z 옵션 처리 (0바이트 파일만 삭제)
    if (opts->zero_only && !is_zero_byte_file(filepath)) {
        if (opts->verbose) {
            printf("skipped '%s' (not a zero-byte file)\n", filepath);
        }
        return 0;  // 0바이트가 아닌 파일은 건너뛰고 성공으로 처리
    }
    
    // 3단계: 디렉토리 처리
    if (is_directory(filepath)) {
        if (!opts->recursive) {
            // recursive 옵션 없이 디렉토리를 삭제하려는 경우 에러
            if (!opts->force) {
                fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", filepath);
            }
            return -1;
        }
        // recursive 옵션이 있으면 재귀적으로 디렉토리 삭제
        return remove_directory_recursive(filepath, opts);
    }
    
    // 4단계: 대화형 확인 (-i 옵션)
    if (opts->interactive && !get_user_confirmation(filepath)) {
        return 0;  // 사용자가 삭제를 거부하면 성공으로 처리 (에러가 아님)
    }
    
    // 5단계: 실제 파일 삭제
    if (unlink(filepath) != 0) {
        // unlink() 시스템 콜 실패시
        if (!opts->force) {
            fprintf(stderr, "rm: cannot remove '%s': %s\n", filepath, strerror(errno));
        }
        // force 모드에서는 삭제 실패도 성공으로 처리
        return opts->force ? 0 : -1;
    }
    
    // 6단계: verbose 출력 (-v 옵션)
    if (opts->verbose) {
        printf("removed '%s'\n", filepath);
    }
    
    return 0;  // 성공
}

/**
 * 메인 함수 - 프로그램 진입점
 * @param argc 명령행 인자 개수
 * @param argv 명령행 인자 배열
 * @return 0: 성공, 1: 실패
 * 
 * 명령행 인자를 파싱하고 각 파일/디렉토리에 대해 삭제를 수행하는 메인 로직
 */
int main(int argc, char *argv[]) {
    rm_options_t opts;           // 옵션 구조체
    int file_start_index;        // 파일 인자가 시작되는 인덱스
    int result = 0;              // 전체 실행 결과 (0: 성공, 1: 실패)
    
    // 1단계: 옵션 구조체 초기화 (모든 옵션을 false로 설정)
    init_options(&opts);
    
    // 2단계: 최소 인자 개수 확인
    if (argc < 2) {
        print_usage(argv[0]);  // 사용법 출력
        return 1;
    }
    
    // 3단계: 명령행 옵션 파싱
    if (parse_options(argc, argv, &opts, &file_start_index) != 0) {
        return 1;  // 옵션 파싱 실패시 프로그램 종료
    }
    
    // 4단계: 삭제할 파일이 지정되었는지 확인
    if (file_start_index >= argc) {
        fprintf(stderr, "rm: missing operand\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // 5단계: 지정된 모든 파일/디렉토리에 대해 삭제 수행
    for (int i = file_start_index; i < argc; i++) {
        if (remove_file(argv[i], &opts) != 0) {
            result = 1;  // 하나라도 실패하면 전체 결과를 실패로 설정
            // 하지만 계속해서 나머지 파일들도 처리함
        }
    }
    
    return result;  // 전체 실행 결과 반환
}