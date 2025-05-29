#include "cat_options.h"

/**
 * cat_file - 파일의 내용을 읽어서 표준 출력으로 출력하는 함수
 * @fp: 읽을 파일 포인터 (파일 또는 stdin)
 * @opts: cat 명령어 옵션들을 담은 구조체 포인터
 * 
 * 기능:
 * - 파일을 한 줄씩 읽어서 출력
 * - -n 옵션: 모든 줄에 번호 표시
 * - -b 옵션: 빈 줄이 아닌 줄에만 번호 표시  
 * - -s 옵션: 연속된 빈 줄을 하나로 압축
 * - -h N 옵션: 처음 N줄만 출력
 * 
 * @return: 성공 시 0 반환
 */
int cat_file(FILE *fp, cat_options_t *opts) {
    char *line = NULL;          // getline()에서 사용할 버퍼 (동적 할당됨)
    size_t len = 0;             // 버퍼 크기
    ssize_t read;               // getline()의 반환값 (읽은 바이트 수)
    int line_num = 1;           // 파일의 실제 줄 번호 (모든 줄 포함)
    int output_line_num = 1;    // 출력할 줄 번호 (-b 옵션용, 빈 줄 제외)
    int prev_blank = 0;         // 이전 줄이 빈 줄이었는지 저장 (-s 옵션용)
    int lines_printed = 0;      // 실제로 출력한 줄의 개수 (-h 옵션용)
    
    // 파일을 한 줄씩 읽어서 처리
    while ((read = getline(&line, &len, fp)) != -1) {
        // -h 옵션 처리: 지정된 줄 수만큼 출력했으면 중단
        if (opts->head_lines > 0 && lines_printed >= opts->head_lines) {
            break;
        }

        // 현재 줄이 빈 줄인지 확인 (공백, 탭, 개행만 있는 줄)
        int is_blank = is_blank_line(line);

        // -s 옵션 처리: 연속된 빈 줄 중 두 번째부터는 건너뛰기
        if (opts->squeeze_blank && is_blank && prev_blank) {
            line_num++;         // 줄 번호는 증가시키되
            continue;           // 출력하지 않고 다음 줄로
        }

        // 줄 번호 출력 처리
        if (opts->number_nonblank && !is_blank) {
            // -b 옵션: 빈 줄이 아닌 경우에만 줄 번호 출력
            printf("%6d\t", output_line_num++);
        } else if (opts->number_all) {
            // -n 옵션: 모든 줄에 줄 번호 출력
            printf("%6d\t", line_num);
        }

        // 실제 줄 내용 출력 (개행 문자 포함)
        printf("%s", line);

        // 다음 반복을 위한 상태 업데이트
        prev_blank = is_blank;  // 현재 줄의 빈 줄 여부를 저장
        line_num++;             // 파일의 실제 줄 번호 증가
        lines_printed++;        // 출력한 줄 수 증가
    }

    // getline()에서 동적 할당된 메모리 해제
    if (line) {
        free(line);
    }

    return 0;
}

/**
 * cat_stdin - 표준 입력에서 데이터를 읽어서 처리하는 함수
 * @opts: cat 명령어 옵션들을 담은 구조체 포인터
 * 
 * 표준 입력(stdin)을 대상으로 cat_file() 함수를 호출하는 래퍼 함수
 * 파일 인자가 없을 때 사용됨
 * 
 * @return: cat_file()의 반환값
 */
int cat_stdin(cat_options_t *opts) {
    return cat_file(stdin, opts);
}

/**
 * main - 프로그램의 진입점
 * @argc: 명령행 인자의 개수
 * @argv: 명령행 인자 배열
 * 
 * 프로그램 실행 순서:
 * 1. 옵션 구조체 초기화
 * 2. 명령행 인자를 파싱하여 옵션 설정
 * 3. 파일이 지정되지 않으면 표준 입력 처리
 * 4. 지정된 파일들을 순서대로 열어서 처리
 * 
 * @return: 성공 시 0, 오류 시 양수
 */
int main(int argc, char *argv[]) {
    cat_options_t opts;       // 명령어 옵션들을 저장할 구조체
    int file_start_idx;       // argv에서 첫 번째 파일명의 인덱스
    int result;               // parse_options()의 반환값

    // 옵션 구조체를 기본값으로 초기화
    init_options(&opts);

    // 명령행 인자를 분석하여 옵션 설정 및 파일 시작 위치 찾기
    result = parse_options(argc, argv, &opts, &file_start_idx);
    if (result == -1) {
        return 0; // --help 옵션으로 도움말 출력 후 정상 종료
    } else if (result != 0) {
        return result; // 옵션 파싱 오류 발생 시 에러 코드와 함께 종료
    }

    // 파일 인자가 없는 경우 표준 입력에서 읽기
    if (file_start_idx >= argc) {
        return cat_stdin(&opts);
    }

    // 지정된 파일들을 하나씩 처리
    for (int i = file_start_idx; i < argc; i++) {
        FILE *fp;

        // "-" 인자는 표준 입력을 의미하는 특수 표기
        if (strcmp(argv[i], "-") == 0) {
            fp = stdin;
        } else {
            // 파일 열기 시도
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                // 파일 열기 실패 시 에러 메시지 출력하고 다음 파일로 계속
                fprintf(stderr, "%s: %s: No such file or directory\n", argv[0], argv[i]);
                continue;
            }
        }

        // 파일 내용 처리
        cat_file(fp, &opts);

        // 표준 입력이 아닌 경우에만 파일 닫기
        if (fp != stdin) {
            fclose(fp);
        }
    }

    return 0;
}