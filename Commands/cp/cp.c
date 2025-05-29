/**
 * cp.c - cp 명령어 구현 (파일 복사 유틸리티)
 * 
 * 이 파일은 UNIX/Linux의 cp 명령어를 구현합니다.
 * 기본적인 파일 복사와 함께 다양한 옵션들을 지원합니다.
 */

#include "cp_options.h"
#include <stdio.h>      // 표준 입출력 (printf, fprintf, fgets 등)
#include <stdlib.h>     // 일반 유틸리티 (exit, malloc 등)
#include <unistd.h>     // POSIX API (access, unlink, close 등)
#include <fcntl.h>      // 파일 제어 (open, O_RDONLY 등)
#include <sys/stat.h>   // 파일 상태 (stat, chmod, struct stat 등)
#include <sys/time.h>   // 시간 관련 구조체
#include <utime.h>      // 파일 시간 변경 (utime, struct utimbuf)
#include <errno.h>      // 에러 번호 (errno, EPERM 등)
#include <string.h>     // 문자열 처리 (strerror 등)
#include <pwd.h>        // 사용자 정보 (사용되지 않음, 확장용)
#include <grp.h>        // 그룹 정보 (사용되지 않음, 확장용)

// 파일 복사 시 사용할 버퍼 크기 (8KB)
// 너무 작으면 시스템 콜이 많아지고, 너무 크면 메모리 낭비
#define BUFFER_SIZE 8192

/**
 * ask_user_confirmation - 사용자에게 덮어쓰기 확인을 요청
 * @dst_path: 덮어쓸 대상 파일의 경로
 * 
 * -i 옵션 사용 시 기존 파일을 덮어쓰기 전에 사용자에게 확인을 받습니다.
 * 표준 입력에서 사용자의 응답을 읽어 'y' 또는 'Y'면 승인으로 처리합니다.
 * 
 * @return: 사용자가 승인하면 1, 거부하거나 입력 오류 시 0
 */
int ask_user_confirmation(const char *dst_path) {
    printf("cp: overwrite '%s'? ", dst_path);
    fflush(stdout); // 즉시 출력되도록 버퍼 플러시
    
    char response[10];  // 사용자 응답을 저장할 버퍼
    if (fgets(response, sizeof(response), stdin) == NULL) {
        return 0; // EOF나 읽기 에러 시 거부로 처리
    }
    
    // 첫 번째 문자가 'y' 또는 'Y'인지 확인
    return (response[0] == 'y' || response[0] == 'Y');
}

/**
 * is_source_newer - 소스 파일이 대상 파일보다 새로운지 확인
 * @src_path: 소스 파일 경로
 * @dst_path: 대상 파일 경로
 * 
 * -u 옵션에서 사용되며, 파일의 수정 시간(mtime)을 비교하여
 * 소스 파일이 더 새로운 경우에만 복사하도록 합니다.
 * 
 * @return: 소스가 더 새로우면 1, 같거나 오래되면 0, 오류 시 -1
 */
int is_source_newer(const char *src_path, const char *dst_path) {
    struct stat src_stat, dst_stat;
    
    // 소스 파일의 상태 정보 가져오기
    if (stat(src_path, &src_stat) != 0) {
        return -1; // 소스 파일 stat 실패
    }
    
    // 대상 파일의 상태 정보 가져오기
    if (stat(dst_path, &dst_stat) != 0) {
        return 1; // 대상 파일이 없으면 무조건 복사해야 함
    }
    
    // 수정 시간 비교 (초 단위)
    return (src_stat.st_mtime > dst_stat.st_mtime);
}

/**
 * preserve_attributes - 소스 파일의 속성을 대상 파일에 복사
 * @src_path: 소스 파일 경로 (속성을 복사할 원본)
 * @dst_path: 대상 파일 경로 (속성을 적용할 파일)
 * 
 * -p 옵션에서 사용되며, 다음 속성들을 보존합니다:
 * - 파일 권한 (chmod)
 * - 소유자 및 그룹 (chown) - 권한이 부족하면 무시
 * - 접근 시간 및 수정 시간 (utime)
 * 
 * @return: 성공 시 0, 실패 시 -1
 */
int preserve_attributes(const char *src_path, const char *dst_path) {
    struct stat src_stat;
    
    // 소스 파일의 속성 정보 가져오기
    if (stat(src_path, &src_stat) != 0) {
        perror("stat");
        return -1;
    }
    
    // 파일 권한 설정 (rwxrwxrwx 형태의 모드)
    if (chmod(dst_path, src_stat.st_mode) != 0) {
        perror("chmod");
        return -1;
    }
    
    // 소유자 및 그룹 설정 (root 권한이 필요할 수 있음)
    if (chown(dst_path, src_stat.st_uid, src_stat.st_gid) != 0) {
        // 소유자 변경 실패는 권한 부족인 경우가 많으므로 경고만 출력
        if (errno != EPERM) {
            perror("chown");
        }
    }
    
    // 파일 시간 설정 (접근 시간, 수정 시간)
    struct utimbuf times;
    times.actime = src_stat.st_atime;   // 마지막 접근 시간
    times.modtime = src_stat.st_mtime;  // 마지막 수정 시간
    
    if (utime(dst_path, &times) != 0) {
        perror("utime");
        return -1;
    }
    
    return 0;
}

/**
 * copy_file_content - 파일의 실제 내용을 복사
 * @src_path: 소스 파일 경로
 * @dst_path: 대상 파일 경로
 * 
 * 파일을 열고 버퍼를 사용하여 내용을 청크 단위로 복사합니다.
 * 대상 파일은 새로 생성되거나 기존 내용이 덮어쓰여집니다.
 * 
 * 처리 과정:
 * 1. 소스 파일을 읽기 전용으로 열기
 * 2. 대상 파일을 쓰기용으로 생성/열기 (기존 내용 삭제)
 * 3. BUFFER_SIZE 만큼씩 읽어서 대상 파일에 쓰기
 * 4. 파일 디스크립터 닫기
 * 
 * @return: 성공 시 0, 실패 시 -1
 */
int copy_file_content(const char *src_path, const char *dst_path) {
    int src_fd, dst_fd;         // 소스, 대상 파일 디스크립터
    char buffer[BUFFER_SIZE];   // 데이터 복사용 버퍼
    ssize_t bytes_read, bytes_written;  // 읽은/쓴 바이트 수
    int result = 0;             // 함수 반환값
    
    // 소스 파일을 읽기 전용으로 열기
    src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "cp: cannot open '%s': %s\n", src_path, strerror(errno));
        return -1;
    }
    
    // 대상 파일을 쓰기용으로 생성/열기
    // O_CREAT: 파일이 없으면 생성, O_TRUNC: 기존 내용 삭제
    // 0644: 소유자는 읽기/쓰기, 그룹/기타는 읽기만 가능
    dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        fprintf(stderr, "cp: cannot create '%s': %s\n", dst_path, strerror(errno));
        close(src_fd);
        return -1;
    }
    
    // 파일 내용을 버퍼 단위로 복사
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            // 쓰기 실패 또는 부분 쓰기 발생
            fprintf(stderr, "cp: error writing to '%s': %s\n", dst_path, strerror(errno));
            result = -1;
            break;
        }
    }
    
    // 읽기 에러 확인
    if (bytes_read == -1) {
        fprintf(stderr, "cp: error reading from '%s': %s\n", src_path, strerror(errno));
        result = -1;
    }
    
    // 파일 디스크립터 닫기
    close(src_fd);
    close(dst_fd);
    
    return result;
}

/**
 * perform_copy - 실제 파일 복사 작업을 수행하는 메인 함수
 * @src_path: 소스 파일 경로
 * @dst_path: 대상 파일 경로  
 * @opts: 복사 옵션들을 담은 구조체
 * 
 * 복사 과정:
 * 1. 소스 파일 접근 가능 여부 확인
 * 2. 대상 파일 존재 여부 확인
 * 3. 옵션에 따른 조건 검사 (-u, -i, -f)
 * 4. 파일 내용 복사 실행
 * 5. 속성 보존 처리 (-p)
 * 
 * @return: 성공 시 0, 실패 시 -1
 */
int perform_copy(const char *src_path, const char *dst_path, const cp_options_t *opts) {
    struct stat dst_stat;
    
    // 소스 파일이 존재하고 읽기 가능한지 확인
    if (access(src_path, R_OK) != 0) {
        fprintf(stderr, "cp: cannot access '%s': %s\n", src_path, strerror(errno));
        return -1;
    }
    
    // 대상 파일의 존재 여부 확인
    int dst_exists = (stat(dst_path, &dst_stat) == 0);
    
    // -u 옵션 처리: 소스가 더 새로운 경우만 복사
    if (opts->update && dst_exists) {
        int newer = is_source_newer(src_path, dst_path);
        if (newer == -1) {
            return -1; // 시간 비교 실패
        }
        if (newer == 0) {
            return 0; // 소스가 더 새롭지 않으므로 복사 생략
        }
    }
    
    // -i 옵션 처리: 덮어쓰기 전 사용자 확인 (-f 옵션이 없는 경우)
    if (opts->interactive && !opts->force && dst_exists) {
        if (!ask_user_confirmation(dst_path)) {
            return 0; // 사용자가 거부했으므로 복사 중단
        }
    }
    
    // -f 옵션 처리: 대상 파일이 쓰기 금지되어 있어도 강제 삭제
    if (opts->force && dst_exists) {
        if (unlink(dst_path) != 0 && errno != ENOENT) {
            fprintf(stderr, "cp: cannot remove '%s': %s\n", dst_path, strerror(errno));
            return -1;
        }
    }
    
    // 실제 파일 내용 복사 실행
    if (copy_file_content(src_path, dst_path) != 0) {
        return -1;
    }
    
    // -p 옵션 처리: 파일 속성 보존
    if (opts->preserve) {
        if (preserve_attributes(src_path, dst_path) != 0) {
            // 속성 보존 실패는 경고만 출력하고 성공으로 처리
            fprintf(stderr, "cp: warning: failed to preserve some attributes for '%s'\n", dst_path);
        }
    }
    
    return 0;
}

/**
 * main - 프로그램의 진입점
 * @argc: 명령행 인자의 개수
 * @argv: 명령행 인자 배열
 * 
 * 프로그램 실행 순서:
 * 1. 옵션 구조체 초기화
 * 2. 명령행 인자 파싱 (옵션 및 파일 경로 추출)
 * 3. 파일 복사 작업 수행
 * 
 * @return: 성공 시 0, 실패 시 1
 */
int main(int argc, char *argv[]) {
    cp_options_t opts;          // 복사 옵션을 저장할 구조체
    char *src_path, *dst_path;  // 소스 및 대상 파일 경로 포인터
    
    // 옵션 구조체를 기본값으로 초기화
    init_options(&opts);
    
    // 명령행 인자를 파싱하여 옵션 설정 및 파일 경로 추출
    if (parse_options(argc, argv, &opts, &src_path, &dst_path) != 0) {
        return 1; // 파싱 실패 시 에러 코드 반환
    }
    
    // 실제 파일 복사 작업 수행
    if (perform_copy(src_path, dst_path, &opts) != 0) {
        return 1; // 복사 실패 시 에러 코드 반환
    }
    
    return 0; // 성공적으로 완료
}