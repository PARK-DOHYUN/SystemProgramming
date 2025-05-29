#include "ls_options.h"

// qsort용 비교 함수 래퍼
// qsort는 옵션 정보를 전달받을 방법이 없어서 전역 변수 사용
static ls_options_t *global_options = NULL;

/**
 * qsort 함수에서 사용하는 비교 함수 래퍼
 * qsort는 사용자 정의 데이터를 전달할 방법이 없어서
 * 전역 변수를 통해 옵션 정보를 compare_files 함수에 전달
 */
int qsort_compare_wrapper(const void *a, const void *b) {
    return compare_files(a, b, global_options);
}

/**
 * 지정된 디렉토리의 내용을 나열하는 메인 함수
 * 파일 정보를 수집하고, 정렬하고, 옵션에 따라 출력
 */
void list_directory(const char *path, ls_options_t *options, int is_recursive) {
    DIR *dir;
    struct dirent *entry;
    file_info_t *files = NULL;  // 동적 배열로 파일 정보 저장
    int file_count = 0;         // 현재 저장된 파일 개수
    int capacity = 0;           // 배열의 현재 용량
    
    // 디렉토리 열기
    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: cannot access '%s': %s\n", path, strerror(errno));
        return;
    }
    
    // 재귀 호출 시에만 디렉토리 이름 출력
    // 최초 호출 시에는 디렉토리명을 출력하지 않음
    if (is_recursive) {
        printf("\n%s:\n", path);
    }
    
    // === 1단계: 파일 정보 수집 ===
    // 디렉토리 내의 모든 항목을 순회하며 파일 정보 수집
    while ((entry = readdir(dir)) != NULL) {
        // 옵션에 따라 파일을 표시할지 검사
        if (!should_show_file(entry->d_name, options)) {
            continue;
        }
        
        // 동적 배열 크기 확장 (필요시)
        if (file_count >= capacity) {
            capacity = capacity == 0 ? 10 : capacity * 2;  // 초기 10개, 이후 2배씩 증가
            files = realloc(files, capacity * sizeof(file_info_t));
            if (!files) {
                fprintf(stderr, "ls: memory allocation failed\n");
                closedir(dir);
                return;
            }
        }
        
        // 파일 정보 저장
        files[file_count].name = strdup(entry->d_name);  // 파일명 복사
        
        // 전체 경로 생성 (디렉토리 경로 + "/" + 파일명)
        files[file_count].path = malloc(strlen(path) + strlen(entry->d_name) + 2);
        sprintf(files[file_count].path, "%s/%s", path, entry->d_name);
        
        // 파일의 상세 정보 (stat) 수집
        if (stat(files[file_count].path, &files[file_count].stat_info) != 0) {
            // stat 실패 시 에러 출력하고 해당 파일 제외
            fprintf(stderr, "ls: cannot stat '%s': %s\n", 
                    files[file_count].path, strerror(errno));
            free(files[file_count].name);
            free(files[file_count].path);
            continue;  // 다음 파일로 계속
        }
        
        file_count++;
    }
    
    closedir(dir);
    
    // 파일이 하나도 없으면 메모리 해제 후 종료
    if (file_count == 0) {
        free(files);
        return;
    }
    
    // === 2단계: 파일 정렬 ===
    // 전역 변수에 옵션 설정 후 qsort 실행
    global_options = options;
    qsort(files, file_count, sizeof(file_info_t), qsort_compare_wrapper);
    
    // === 3단계: 헤더 정보 출력 ===
    // long format에서는 총 블록 수를 먼저 출력
    if (options->long_format) {
        long total_blocks = 0;
        for (int i = 0; i < file_count; i++) {
            total_blocks += files[i].stat_info.st_blocks;
        }
        printf("total %ld\n", total_blocks / 2); // 512바이트 블록을 1K 블록으로 변환
    }
    
    // === 4단계: 파일 정보 출력 ===
    // 정렬된 순서대로 각 파일의 정보를 출력
    for (int i = 0; i < file_count; i++) {
        print_file_info(&files[i], options);
    }
    
    // === 5단계: 재귀 처리 ===
    // 재귀 옵션이 설정된 경우, 하위 디렉토리들을 재귀적으로 처리
    if (options->recursive) {
        for (int i = 0; i < file_count; i++) {
            // 디렉토리이면서 현재/상위 디렉토리가 아닌 경우만 재귀 호출
            if (S_ISDIR(files[i].stat_info.st_mode) && 
                strcmp(files[i].name, ".") != 0 && 
                strcmp(files[i].name, "..") != 0) {
                list_directory(files[i].path, options, 1);  // is_recursive=1로 호출
            }
        }
    }
    
    // === 6단계: 메모리 정리 ===
    // 동적으로 할당된 모든 메모리 해제
    free_file_info_array(files, file_count);
}

/**
 * 프로그램의 진입점 (main 함수)
 * 명령행 인자를 파싱하고 디렉토리 나열을 실행
 */
int main(int argc, char *argv[]) {
    ls_options_t options;           // 옵션 정보를 저장할 구조체
    const char *directory = ".";    // 기본 디렉토리는 현재 디렉토리
    
    // === 1단계: 명령행 옵션 파싱 ===
    parse_options(argc, argv, &options);
    
    // === 2단계: 디렉토리 인자 처리 ===
    // 옵션이 아닌 인자가 있으면 그것을 디렉토리 경로로 사용
    if (optind < argc) {
        directory = argv[optind];  // optind는 getopt에서 설정하는 다음 인자 인덱스
    }
    
    // === 3단계: 디렉토리 나열 실행 ===
    // is_recursive=0으로 설정하여 최초 호출임을 표시
    list_directory(directory, &options, 0);
    
    // === 4단계: 메모리 정리 ===
    // 확장자 필터 문자열이 동적 할당되었다면 해제
    if (options.filter_ext) {
        free(options.filter_ext);
    }
    
    return 0;  // 정상 종료
}