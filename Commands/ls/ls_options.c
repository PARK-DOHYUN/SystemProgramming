#include "ls_options.h"

/**
 * 명령행 인자를 파싱하여 ls 옵션을 설정하는 함수
 * getopt를 사용하여 Unix 스타일의 옵션 파싱을 수행
 */
void parse_options(int argc, char *argv[], ls_options_t *options) {
    // 옵션 구조체를 모두 0으로 초기화 (모든 플래그를 false로 설정)
    memset(options, 0, sizeof(ls_options_t));
    
    int opt;
    
    // getopt를 사용한 옵션 파싱
    // 문자열 "alhtsrRiedf:"는 허용되는 옵션들을 정의
    // 콜론(:)이 붙은 옵션(f:)은 인자를 받음
    while ((opt = getopt(argc, argv, "alhtsrRiedf:")) != -1) {
        switch (opt) {
            case 'a':   // 숨김 파일 표시 옵션
                options->show_all = 1;
                break;
            case 'l':   // 상세 정보 표시 옵션
                options->long_format = 1;
                break;
            case 'h':   // 사람이 읽기 쉬운 크기 표시 옵션
                options->human_readable = 1;
                break;
            case 't':   // 시간순 정렬 옵션
                options->sort_by_time = 1;
                break;
            case 's':   // 블록 크기 표시 옵션
                options->show_size = 1;
                break;
            case 'r':   // 역순 정렬 옵션
                options->reverse_sort = 1;
                break;
            case 'R':   // 재귀적 디렉토리 탐색 옵션
                options->recursive = 1;
                break;
            case 'i':   // inode 번호 표시 옵션
                options->show_inode = 1;
                break;
            case 'e':   // 확장자별 그룹화 옵션
                options->group_by_ext = 1;
                break;
            case 'd':   // 디렉토리 우선 표시 옵션
                options->dirs_first = 1;
                break;
            case 'f':   // 확장자 필터 옵션 (인자 필요)
                if (optarg) {
                    // optarg는 -f 옵션 뒤에 오는 확장자 문자열
                    options->filter_ext = strdup(optarg);
                }
                break;
            case '?':   // 알 수 없는 옵션이나 잘못된 사용
                print_usage(argv[0]);
                exit(1);
        }
    }
}

/**
 * 프로그램 사용법을 표준 출력으로 출력하는 함수
 */
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [DIRECTORY]\n", program_name);
    printf("Options:\n");
    printf("  -a    Show hidden files\n");
    printf("  -l    Use long listing format\n");
    printf("  -h    Human-readable file sizes\n");
    printf("  -t    Sort by modification time\n");
    printf("  -s    Show file size in blocks\n");
    printf("  -r    Reverse sort order\n");
    printf("  -R    List subdirectories recursively\n");
    printf("  -i    Show inode numbers\n");
    printf("  -e    Group by file extension\n");
    printf("  -d    List directories first\n");
    printf("  -f EXT Filter by file extension\n");
}

/**
 * 두 파일을 비교하여 정렬 순서를 결정하는 함수
 * 다양한 옵션에 따라 비교 기준이 달라짐
 */
int compare_files(const void *a, const void *b, ls_options_t *options) {
    file_info_t *file_a = (file_info_t *)a;
    file_info_t *file_b = (file_info_t *)b;
    int result = 0;
    
    // 1단계: 디렉토리 우선 정렬 (옵션이 설정된 경우)
    if (options->dirs_first) {
        // 각 파일이 디렉토리인지 확인
        int a_is_dir = S_ISDIR(file_a->stat_info.st_mode);
        int b_is_dir = S_ISDIR(file_b->stat_info.st_mode);
        
        // 한쪽은 디렉토리, 다른 쪽은 일반 파일인 경우
        if (a_is_dir && !b_is_dir) return -1;  // a가 앞에 옴
        if (!a_is_dir && b_is_dir) return 1;   // b가 앞에 옴
    }
    
    // 2단계: 확장자별 그룹화 (옵션이 설정된 경우)
    if (options->group_by_ext) {
        char *ext_a = get_file_extension(file_a->name);
        char *ext_b = get_file_extension(file_b->name);
        
        // 둘 다 확장자가 있는 경우: 확장자 비교
        if (ext_a && ext_b) {
            result = strcmp(ext_a, ext_b);
            if (result != 0) {
                return options->reverse_sort ? -result : result;
            }
        } 
        // 한쪽만 확장자가 있는 경우: 확장자 있는 것을 뒤로
        else if (ext_a && !ext_b) {
            return options->reverse_sort ? -1 : 1;
        } else if (!ext_a && ext_b) {
            return options->reverse_sort ? 1 : -1;
        }
        // 둘 다 확장자가 없으면 다음 단계로
    }
    
    // 3단계: 시간 기준 정렬 vs 이름 기준 정렬
    if (options->sort_by_time) {
        // 수정 시간 비교 (최신 파일이 앞에 오도록)
        if (file_a->stat_info.st_mtime < file_b->stat_info.st_mtime) {
            result = -1;
        } else if (file_a->stat_info.st_mtime > file_b->stat_info.st_mtime) {
            result = 1;
        } else {
            result = 0;
        }
        
        // 시간이 같으면 이름으로 보조 정렬
        if (result == 0) {
            result = strcmp(file_a->name, file_b->name);
        }
    } else {
        // 기본: 파일명 기준 알파벳 순 정렬
        result = strcmp(file_a->name, file_b->name);
    }
    
    // 4단계: 역순 정렬 옵션 적용
    return options->reverse_sort ? -result : result;
}

/**
 * 단일 파일의 정보를 설정된 형식에 맞춰 출력하는 함수
 */
void print_file_info(file_info_t *file, ls_options_t *options) {
    // inode 번호 출력 (옵션이 설정된 경우)
    if (options->show_inode) {
        printf("%8lu ", (unsigned long)file->stat_info.st_ino);
    }
    
    // 블록 단위 크기 출력 (옵션이 설정된 경우)
    if (options->show_size) {
        printf("%8ld ", (long)file->stat_info.st_blocks);
    }
    
    // 상세 정보 출력 (long format 옵션이 설정된 경우)
    if (options->long_format) {
        // 파일 권한 문자열 생성 및 출력
        char permissions[11];
        format_permissions(file->stat_info.st_mode, permissions);
        printf("%s ", permissions);
        
        // 하드링크 수 출력
        printf("%3lu ", (unsigned long)file->stat_info.st_nlink);
        
        // 소유자 및 그룹 정보 조회 및 출력
        struct passwd *pwd = getpwuid(file->stat_info.st_uid);
        struct group *grp = getgrgid(file->stat_info.st_gid);
        
        printf("%-8s %-8s ", 
               pwd ? pwd->pw_name : "unknown",    // 소유자명 (또는 "unknown")
               grp ? grp->gr_name : "unknown");   // 그룹명 (또는 "unknown")
        
        // 파일 크기 포맷팅 및 출력
        char size_str[20];
        format_size(file->stat_info.st_size, size_str, options->human_readable);
        printf("%8s ", size_str);
        
        // 수정 시간 포맷팅 및 출력
        char *time_str = ctime(&file->stat_info.st_mtime);
        time_str[strlen(time_str) - 1] = '\0'; // 개행 문자 제거
        printf("%.12s ", time_str + 4);        // "Mon DD HH:MM" 형식으로 출력
    }
    
    // 파일명 출력 (항상 마지막에)
    printf("%s\n", file->name);
}

/**
 * 파일 크기를 적절한 형식으로 포맷팅하는 함수
 */
void format_size(off_t size, char *buffer, int human_readable) {
    // 사람이 읽기 쉬운 형식이 요청되고 크기가 1KB 이상인 경우
    if (human_readable && size >= 1024) {
        const char *units[] = {"", "K", "M", "G", "T"};  // 단위 배열
        int unit_index = 0;
        double size_d = (double)size;
        
        // 적절한 단위까지 1024로 나누기 반복
        while (size_d >= 1024.0 && unit_index < 4) {
            size_d /= 1024.0;
            unit_index++;
        }
        
        // 크기에 따라 소수점 표시 여부 결정
        if (size_d < 10.0) {
            snprintf(buffer, 20, "%.1f%s", size_d, units[unit_index]);  // 소수점 1자리
        } else {
            snprintf(buffer, 20, "%.0f%s", size_d, units[unit_index]);  // 정수
        }
    } else {
        // 기본: 바이트 단위로 표시
        snprintf(buffer, 20, "%ld", (long)size);
    }
}

/**
 * 파일의 mode 비트를 권한 문자열로 변환하는 함수
 * 결과 형식: drwxrwxrwx (파일 타입 + 소유자/그룹/기타 권한)
 */
void format_permissions(mode_t mode, char *buffer) {
    // 첫 번째 문자: 파일 타입 결정
    buffer[0] = S_ISDIR(mode) ? 'd' :      // 디렉토리
                S_ISLNK(mode) ? 'l' :      // 심볼릭 링크
                S_ISCHR(mode) ? 'c' :      // 문자 디바이스
                S_ISBLK(mode) ? 'b' :      // 블록 디바이스
                S_ISFIFO(mode) ? 'p' :     // 파이프
                S_ISSOCK(mode) ? 's' : '-'; // 소켓 또는 일반 파일
    
    // 소유자 권한 (2-4번째 문자)
    buffer[1] = (mode & S_IRUSR) ? 'r' : '-';  // 읽기
    buffer[2] = (mode & S_IWUSR) ? 'w' : '-';  // 쓰기
    buffer[3] = (mode & S_IXUSR) ? 'x' : '-';  // 실행
    
    // 그룹 권한 (5-7번째 문자)
    buffer[4] = (mode & S_IRGRP) ? 'r' : '-';  // 읽기
    buffer[5] = (mode & S_IWGRP) ? 'w' : '-';  // 쓰기
    buffer[6] = (mode & S_IXGRP) ? 'x' : '-';  // 실행
    
    // 기타 사용자 권한 (8-10번째 문자)
    buffer[7] = (mode & S_IROTH) ? 'r' : '-';  // 읽기
    buffer[8] = (mode & S_IWOTH) ? 'w' : '-';  // 쓰기
    buffer[9] = (mode & S_IXOTH) ? 'x' : '-';  // 실행
    
    buffer[10] = '\0';  // 문자열 종료
}

/**
 * 파일명에서 확장자 부분을 추출하는 함수
 */
char *get_file_extension(const char *filename) {
    // 뒤에서부터 점(.) 문자를 찾음
    char *dot = strrchr(filename, '.');
    
    // 점이 없거나 파일명이 점으로 시작하는 경우 (숨김 파일)
    if (!dot || dot == filename) return NULL;
    
    // 점 다음 문자부터가 확장자
    return dot + 1;
}

/**
 * 설정된 옵션에 따라 해당 파일을 표시할지 결정하는 함수
 */
int should_show_file(const char *filename, ls_options_t *options) {
    // 숨김 파일 체크: 점으로 시작하는 파일 처리
    if (!options->show_all && filename[0] == '.') {
        return 0;  // 숨김 파일 표시 옵션이 없으면 숨김
    }
    
    // 확장자 필터 체크: 특정 확장자만 표시하는 옵션 처리
    if (options->filter_ext) {
        char *ext = get_file_extension(filename);
        // 확장자가 없거나 지정된 확장자와 다르면 숨김
        if (!ext || strcmp(ext, options->filter_ext) != 0) {
            return 0;
        }
    }
    
    return 1;  // 모든 조건을 통과하면 표시
}

/**
 * 파일 정보 배열의 모든 메모리를 해제하는 함수
 * 메모리 누수 방지를 위해 동적 할당된 모든 메모리를 정리
 */
void free_file_info_array(file_info_t *files, int count) {
    for (int i = 0; i < count; i++) {
        free(files[i].name);  // 각 파일의 이름 문자열 해제
        free(files[i].path);  // 각 파일의 경로 문자열 해제
    }
    free(files);  // 파일 정보 배열 자체 해제
}