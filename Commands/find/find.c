/*
 * find.c - Unix find 명령어 구현 (수정본)
 * 
 * 이 프로그램은 Unix의 find 명령어와 유사한 기능을 제공합니다.
 * 지정된 디렉토리에서 다양한 조건에 맞는 파일과 디렉토리를 재귀적으로 검색합니다.
 * 
 * 주요 기능:
 * - 파일 타입별 검색 (파일, 디렉토리)
 * - 이름 패턴 매칭 (대소문자 구분/무시)
 * - 파일 크기 조건 검색
 * - 소유자별 검색
 * - 권한별 검색
 * - 수정 시간별 검색
 * - 빈 파일/디렉토리 검색
 */

#include "find_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <fnmatch.h>
#include <time.h>
#include <limits.h>

// PATH_MAX가 정의되지 않은 시스템을 위한 대체값
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * 크기 문자열을 바이트 단위로 파싱
 * 
 * @param size_spec 크기 문자열 (예: "100", "+1k", "-5M")
 *                  접두어: +는 보다 큰, -는 보다 작은
 *                  접미어: k/K(킬로바이트), M(메가바이트), G(기가바이트)
 * @return 파싱된 바이트 크기, 실패시 -1
 */
long parse_size_spec(const char *size_spec) {
    if (!size_spec) return -1;
    
    // +/- 접두어를 건너뛰고 숫자 부분 파싱
    char *endptr;
    long size = strtol(size_spec + (size_spec[0] == '+' || size_spec[0] == '-' ? 1 : 0), &endptr, 10);
    
    // 단위 접미어 처리
    if (*endptr == 'k' || *endptr == 'K') {
        size *= 1024;  // 킬로바이트
    } else if (*endptr == 'M') {
        size *= 1024 * 1024;  // 메가바이트
    } else if (*endptr == 'G') {
        size *= 1024 * 1024 * 1024;  // 기가바이트
    }
    
    return size;
}

/**
 * 권한 문자열을 8진수 모드로 변환
 * 
 * @param perm_spec 8진수 권한 문자열 (예: "755", "644")
 * @return 변환된 mode_t 값
 */
mode_t parse_perm_spec(const char *perm_spec) {
    if (!perm_spec) return 0;
    
    // 8진수 문자열을 mode_t로 변환
    return (mode_t)strtol(perm_spec, NULL, 8);
}

/**
 * 사용자 이름으로부터 UID 획득
 * 
 * @param username 사용자 이름
 * @return 해당하는 UID, 실패시 (uid_t)-1
 */
uid_t get_uid_by_name(const char *username) {
    struct passwd *pwd = getpwnam(username);  // 패스워드 데이터베이스에서 사용자 정보 검색
    return pwd ? pwd->pw_uid : (uid_t)-1;
}

/**
 * 파일이나 디렉토리가 비어있는지 확인
 * 
 * @param path 파일/디렉토리 경로
 * @param st 파일 상태 정보 구조체
 * @return 비어있으면 true, 아니면 false
 */
bool is_empty(const char *path, const struct stat *st) {
    if (S_ISREG(st->st_mode)) {
        // 일반 파일의 경우: 크기가 0이면 빈 파일
        return st->st_size == 0;
    } else if (S_ISDIR(st->st_mode)) {
        // 디렉토리의 경우: 하위 항목이 없으면 빈 디렉토리
        DIR *dir = opendir(path);
        if (!dir) return false;
        
        struct dirent *entry;
        int count = 0;
        
        // 디렉토리 내용을 읽어서 실제 파일/디렉토리가 있는지 확인
        while ((entry = readdir(dir)) != NULL) {
            // "."과 ".." 제외하고 실제 항목이 있는지 확인
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                count++;
                break;  // 하나라도 있으면 빈 디렉토리가 아님
            }
        }
        closedir(dir);
        return count == 0;
    }
    return false;
}

/**
 * 파일이 지정된 검색 조건에 맞는지 확인
 * 
 * @param filepath 파일의 전체 경로
 * @param filename 파일명만 (경로 제외)
 * @param st 파일 상태 정보
 * @param opts 검색 옵션 구조체
 * @return 조건에 맞으면 true, 아니면 false
 */
bool matches_criteria(const char *filepath, const char *filename, const struct stat *st, const find_options_t *opts) {
    // 파일 타입 필터링
    if (opts->type_file && !S_ISREG(st->st_mode)) {
        return false;  // 파일만 검색하는데 파일이 아님
    }
    if (opts->type_dir && !S_ISDIR(st->st_mode)) {
        return false;  // 디렉토리만 검색하는데 디렉토리가 아님
    }
    
    // 파일명 패턴 매칭 (대소문자 구분)
    if (opts->name_pattern && fnmatch(opts->name_pattern, filename, 0) != 0) {
        return false;  // 이름 패턴이 일치하지 않음
    }
    
    // 파일명 패턴 매칭 (대소문자 무시)
    if (opts->iname_pattern && fnmatch(opts->iname_pattern, filename, FNM_CASEFOLD) != 0) {
        return false;  // 이름 패턴이 일치하지 않음 (대소문자 무시)
    }
    
    // 파일 크기 조건 확인
    if (opts->size_spec) {
        long target_size = parse_size_spec(opts->size_spec);
        if (target_size < 0) return false;  // 크기 파싱 실패
        
        // 크기 비교 (+: 보다 큰, -: 보다 작은, 없음: 정확히 같은)
        if (opts->size_spec[0] == '+' && st->st_size <= target_size) {
            return false;  // 지정 크기보다 크지 않음
        } else if (opts->size_spec[0] == '-' && st->st_size >= target_size) {
            return false;  // 지정 크기보다 작지 않음
        } else if (opts->size_spec[0] != '+' && opts->size_spec[0] != '-' && st->st_size != target_size) {
            return false;  // 정확한 크기가 아님
        }
    }
    
    // 파일 소유자 확인
    if (opts->user_name) {
        uid_t target_uid = get_uid_by_name(opts->user_name);
        if (target_uid == (uid_t)-1 || st->st_uid != target_uid) {
            return false;  // 지정된 사용자의 파일이 아님
        }
    }
    
    // 파일 권한 확인
    if (opts->perm_spec) {
        mode_t target_perm = parse_perm_spec(opts->perm_spec);
        // 파일 권한 부분만 비교 (하위 9비트: rwxrwxrwx)
        if ((st->st_mode & 0777) != target_perm) {
            return false;  // 권한이 일치하지 않음
        }
    }
    
    // 수정 시간 확인 (+n: n일 이전, -n: n일 이내, n: 정확히 n일 전)
    if (opts->mtime_set) {
        time_t now = time(NULL);
        time_t file_time = st->st_mtime;
        // 현재 시간과 파일 수정 시간의 차이를 일 단위로 계산
        int days_diff = (now - file_time) / (24 * 60 * 60);
        
        // mtime 조건 확인
        if (opts->mtime_prefix == '+' && days_diff <= opts->mtime_days) {
            return false;  // +n: n일보다 이전에 수정된 파일이 아님
        } else if (opts->mtime_prefix == '-' && days_diff >= opts->mtime_days) {
            return false;  // -n: n일 이내에 수정된 파일이 아님
        } else if (opts->mtime_prefix == 0 && days_diff != opts->mtime_days) {
            return false;  // n: 정확히 n일 전에 수정된 파일이 아님
        }
    }
    
    // 빈 파일/디렉토리 필터
    if (opts->empty_filter && !is_empty(filepath, st)) {
        return false;  // 빈 파일/디렉토리가 아님
    }
    
    return true;  // 모든 조건에 부합
}

/**
 * 지정된 경로에서 재귀적으로 파일/디렉토리 검색
 * 
 * @param path 검색할 경로
 * @param opts 검색 옵션
 */
void find_recursive(const char *path, const find_options_t *opts) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char filepath[PATH_MAX];  // 전체 경로 저장용
    
    // 현재 경로 자체도 조건 검사 대상
    if (stat(path, &st) == 0) {
        // 경로에서 파일명만 추출
        const char *filename = strrchr(path, '/');
        filename = filename ? filename + 1 : path;  // '/' 이후 부분, 없으면 전체
        
        // 조건에 맞으면 출력
        if (matches_criteria(path, filename, &st, opts)) {
            printf("%s\n", path);
        }
    }
    
    // 디렉토리가 아니면 더 이상 탐색할 필요 없음
    if (!S_ISDIR(st.st_mode)) {
        return;
    }
    
    // 디렉토리 열기 시도
    dir = opendir(path);
    if (!dir) {
        perror(path);  // 디렉토리 열기 실패시 에러 출력
        return;
    }
    
    // 디렉토리 내의 모든 항목 순회
    while ((entry = readdir(dir)) != NULL) {
        // 현재 디렉토리(.)와 상위 디렉토리(..) 건너뛰기
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 전체 경로 구성 (부모경로/항목명)
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        // 파일 정보 획득 (심볼릭 링크는 링크 자체 정보)
        if (lstat(filepath, &st) != 0) {
            continue;  // 파일 정보 획득 실패시 건너뛰기
        }
        
        // 검색 조건에 맞는지 확인하고 맞으면 출력
        if (matches_criteria(filepath, entry->d_name, &st, opts)) {
            printf("%s\n", filepath);
        }
        
        // 디렉토리인 경우 재귀적으로 하위 탐색
        if (S_ISDIR(st.st_mode)) {
            find_recursive(filepath, opts);
        }
    }
    
    closedir(dir);  // 디렉토리 핸들 정리
}

/**
 * 메인 함수
 * 명령행 인자를 파싱하고 검색을 실행합니다.
 */
int main(int argc, char *argv[]) {
    find_options_t opts;
    
    // 옵션 구조체 초기화
    init_options(&opts);
    
    // 명령행 인자 파싱
    int parse_result = parse_options(argc, argv, &opts);
    if (parse_result != 0) {
        if (parse_result == 1) {
            // 도움말 출력 후 정상 종료
            free_options(&opts);
            return 0;
        }
        // 파싱 오류 발생
        free_options(&opts);
        return 1;
    }
    
    // 지정된 경로에서 검색 시작
    find_recursive(opts.search_path, &opts);
    
    // 메모리 정리
    free_options(&opts);
    return 0;
}