/**
 * cp_options.h - cp 명령어 옵션 처리를 위한 헤더 파일
 * 
 * 이 헤더 파일은 cp 명령어의 옵션들을 정의하고
 * 관련 함수들을 선언합니다.
 */

#ifndef CP_OPTIONS_H
#define CP_OPTIONS_H

// 불린 타입 사용을 위한 표준 라이브러리
#include <stdbool.h>    // bool, true, false 정의

/**
 * cp_options_t - cp 명령어의 모든 옵션을 저장하는 구조체
 * 
 * 각 멤버는 해당 옵션의 활성화 여부를 나타내며,
 * true는 활성화, false는 비활성화를 의미합니다.
 */
typedef struct {
    bool interactive;  // -i 옵션: 기존 파일 덮어쓰기 전 사용자 확인 요청
                      // true면 덮어쓰기 전에 "overwrite 'file'?" 메시지 출력
    
    bool force;        // -f 옵션: 강제 복사 실행
                      // true면 쓰기 금지된 파일도 강제로 삭제하고 복사
                      // -i 옵션과 함께 사용 시 -f가 우선 (확인하지 않음)
    
    bool update;       // -u 옵션: 조건부 복사 (업데이트 모드)
                      // true면 소스 파일이 대상 파일보다 새로운 경우에만 복사
                      // 파일의 수정 시간(mtime)을 비교하여 판단
    
    bool preserve;     // -p 옵션: 파일 속성 보존
                      // true면 복사 후 원본 파일의 속성들을 대상 파일에 적용
                      // 보존되는 속성: 권한(mode), 소유자(uid/gid), 시간(atime/mtime)
} cp_options_t;

// 함수 선언부

/**
 * init_options - 옵션 구조체를 기본값으로 초기화
 * @opts: 초기화할 cp_options_t 구조체 포인터
 * 
 * 모든 옵션을 비활성화 상태(false)로 설정합니다.
 * 프로그램 시작 시 반드시 호출되어야 합니다.
 */
void init_options(cp_options_t *opts);

/**
 * parse_options - 명령행 인자를 파싱하여 옵션과 파일 경로 추출
 * @argc: main 함수의 argc (명령행 인자 개수)
 * @argv: main 함수의 argv (명령행 인자 배열)
 * @opts: 파싱된 옵션을 저장할 구조체 포인터
 * @src: 소스 파일 경로를 저장할 문자열 포인터의 포인터
 * @dst: 대상 파일 경로를 저장할 문자열 포인터의 포인터
 * 
 * 처리 과정:
 * 1. '-'로 시작하는 인자들을 옵션으로 파싱
 * 2. 묶음 옵션 지원 (예: -ifu)
 * 3. 옵션이 아닌 첫 두 개 인자를 소스/대상 파일로 설정
 * 4. 인자 개수 검증 (정확히 2개의 파일 경로 필요)
 * 
 * @return: 성공 시 0, 오류 시 -1
 */
int parse_options(int argc, char *argv[], cp_options_t *opts, char **src, char **dst);

/**
 * print_usage - 프로그램 사용법을 표준 출력으로 출력
 * @program_name: 프로그램 이름 (보통 argv[0])
 * 
 * 잘못된 사용법이나 인자 오류 시 호출되어
 * 올바른 사용법과 지원하는 옵션들을 안내합니다.
 * 
 * 출력 내용:
 * - 기본 사용법 (프로그램명 [옵션] 소스 대상)
 * - 각 옵션의 기능 설명
 * - 옵션 결합 사용법 예시
 */
void print_usage(const char *program_name);

#endif // CP_OPTIONS_H