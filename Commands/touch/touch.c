#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <errno.h>
#include <libgen.h>
#include "touch_options.h"

/**
 * 디렉토리 생성 함수 (재귀적)
 * 지정된 경로의 모든 상위 디렉토리를 재귀적으로 생성한다.
 * 이미 존재하는 디렉토리는 건너뛴다.
 * 
 * @param path 생성할 디렉토리 경로
 * @return 성공 시 0, 실패 시 -1
 */
int create_directories(const char *path) {
    struct stat st;
    
    // 이미 존재하는 디렉토리인지 확인
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    
    // 경로 문자열을 복사 (dirname()이 원본을 수정할 수 있으므로)
    char *path_copy = strdup(path);
    if (!path_copy) {
        perror("메모리 할당 실패");
        return -1;
    }
    
    // 상위 디렉토리 경로 추출
    char *parent_dir = dirname(path_copy);
    
    // 루트 디렉토리이거나 현재 디렉토리가 아닌 경우 재귀적으로 상위 디렉토리 생성
    if (strcmp(parent_dir, "/") != 0 && strcmp(parent_dir, ".") != 0) {
        if (create_directories(parent_dir) != 0) {
            free(path_copy);
            return -1;
        }
    }
    
    free(path_copy);
    
    // 현재 디렉토리 생성 (권한: 755)
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        perror("디렉토리 생성 실패");
        return -1;
    }
    
    return 0;
}

/**
 * 빈 파일 생성 함수
 * 지정된 이름으로 빈 파일을 생성한다.
 * 
 * @param filename 생성할 파일명
 * @return 성공 시 0, 실패 시 -1
 */
int create_file(const char *filename) {
    // 파일 생성 (쓰기 모드, 권한: 644)
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

/**
 * 파일의 접근/수정 시간 설정 함수
 * 옵션에 따라 파일의 접근 시간과 수정 시간을 변경한다.
 * 
 * @param filename 시간을 변경할 파일명
 * @param opts 설정 옵션 구조체
 * @return 성공 시 0, 실패 시 -1
 */
int set_file_times(const char *filename, const touch_options *opts) {
    struct stat file_stat;
    
    // 파일의 현재 정보를 가져온다 (기존 시간 정보 필요)
    if (stat(filename, &file_stat) != 0) {
        return -1;
    }
    
    struct utimbuf times;
    
    if (opts->use_custom_time) {
        // 사용자가 지정한 시간 사용
        if (opts->access_time && opts->modify_time) {
            // 둘 다 변경
            times.actime = opts->custom_time.tv_sec;
            times.modtime = opts->custom_time.tv_sec;
        } else if (opts->access_time) {
            // 접근 시간만 변경, 수정 시간은 기존 값 유지
            times.actime = opts->custom_time.tv_sec;
            times.modtime = file_stat.st_mtime;
        } else if (opts->modify_time) {
            // 수정 시간만 변경, 접근 시간은 기존 값 유지
            times.actime = file_stat.st_atime;
            times.modtime = opts->custom_time.tv_sec;
        }
    } else {
        // 현재 시간 사용
        time_t now = time(NULL);
        if (opts->access_time && opts->modify_time) {
            // 둘 다 현재 시간으로 설정
            times.actime = now;
            times.modtime = now;
        } else if (opts->access_time) {
            // 접근 시간만 현재 시간으로 설정
            times.actime = now;
            times.modtime = file_stat.st_mtime;
        } else if (opts->modify_time) {
            // 수정 시간만 현재 시간으로 설정
            times.actime = file_stat.st_atime;
            times.modtime = now;
        }
    }
    
    // 파일 시간 변경 적용
    return utime(filename, &times);
}

/**
 * 단일 파일 처리 함수
 * 하나의 파일에 대해 전체 touch 작업을 수행한다.
 * 파일이 없으면 생성하고, 시간을 설정한다.
 * 
 * @param filename 처리할 파일명
 * @param opts 설정 옵션 구조체
 * @return 성공 시 0, 실패 시 -1
 */
int process_file(const char *filename, const touch_options *opts) {
    struct stat file_stat;
    // 파일 존재 여부 확인
    int file_exists = (stat(filename, &file_stat) == 0);
    
    // 파일이 존재하지 않는 경우의 처리
    if (!file_exists) {
        if (opts->no_create) {
            // -c 옵션: 파일을 생성하지 않고 종료
            return 0;
        }
        
        // -p 옵션: 필요한 중간 디렉토리들을 생성
        if (opts->create_path) {
            char *filename_copy = strdup(filename);
            if (!filename_copy) {
                perror("메모리 할당 실패");
                return -1;
            }
            
            // 파일의 디렉토리 부분만 추출하여 디렉토리 생성
            char *dir_path = dirname(filename_copy);
            
            // 현재 디렉토리가 아닌 경우에만 디렉토리 생성
            if (strcmp(dir_path, ".") != 0) {
                // 전체 디렉토리 경로를 다시 구성해서 재귀적으로 생성
                char *full_dir_copy = strdup(filename);
                if (!full_dir_copy) {
                    perror("메모리 할당 실패");
                    free(filename_copy);
                    return -1;
                }
                
                char *full_dir_path = dirname(full_dir_copy);
                if (create_directories(full_dir_path) != 0) {
                    free(filename_copy);
                    free(full_dir_copy);
                    return -1;
                }
                free(full_dir_copy);
            }
            free(filename_copy);
        }
        
        // 새 파일 생성
        if (create_file(filename) != 0) {
            perror(filename);
            return -1;
        }
    }
    
    // 파일의 접근/수정 시간 설정
    if (set_file_times(filename, opts) != 0) {
        perror(filename);
        return -1;
    }
    
    return 0;
}

/**
 * touch 프로그램의 메인 함수
 * 명령행 인수를 파싱하고 각 파일에 대해 touch 작업을 수행한다.
 * 
 * @param argc 명령행 인수 개수
 * @param argv 명령행 인수 배열
 * @return 성공 시 0, 실패 시 1
 */
int main(int argc, char *argv[]) {
    touch_options opts;  // 옵션 저장 구조체
    char **files;        // 처리할 파일 목록
    int file_count;      // 파일 개수
    int result = 0;      // 최종 반환값
    
    // 옵션 구조체를 기본값으로 초기화
    init_options(&opts);
    
    // 인수가 없으면 사용법 출력 후 종료
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 명령행 옵션 및 파일명 파싱
    if (parse_options(argc, argv, &opts, &files, &file_count) != 0) {
        return 1;
    }
    
    // 처리할 파일이 지정되지 않은 경우
    if (file_count == 0) {
        fprintf(stderr, "touch: 파일 인수가 누락되었습니다\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // 각 파일에 대해 순차적으로 touch 작업 수행
    // 하나의 파일에서 오류가 발생해도 나머지 파일들은 계속 처리
    for (int i = 0; i < file_count; i++) {
        if (process_file(files[i], &opts) != 0) {
            result = 1; // 오류 발생을 기록하지만 처리는 계속
        }
    }
    
    return result;
}