#ifndef MV_OPTIONS_H
#define MV_OPTIONS_H

#include <stdbool.h>

/**
 * mv 명령어의 모든 옵션을 관리하는 구조체
 * 
 * 이 구조체는 명령줄에서 받은 옵션들의 상태를 저장하고,
 * 프로그램 전체에서 일관된 동작을 보장하기 위해 사용된다.
 * 각 필드는 해당 옵션의 활성화 여부를 나타내는 불린 값이다.
 */
typedef struct {
    bool interactive;   // -i 옵션: 덮어쓰기 전 사용자에게 확인 요청
                       // true일 때 파일 덮어쓰기 전 "덮어쓰시겠습니까?" 메시지 출력
    
    bool force;        // -f 옵션: 강제 이동/덮어쓰기
                       // true일 때 권한이나 확인 과정 없이 강제로 실행
                       // -i 옵션과 함께 사용 시 이 옵션이 우선함
    
    bool no_clobber;   // -n 옵션: 기존 파일 덮어쓰기 방지
                       // true일 때 이미 존재하는 파일을 절대 덮어쓰지 않음
                       // -f 옵션과 상호 배타적 (둘 다 있으면 -f가 우선)
    
    bool verbose;      // -v 옵션: 상세 정보 출력 모드
                       // true일 때 "파일A -> 파일B" 형태로 이동 과정 출력
                       // 디버깅이나 스크립트에서 진행 상황 확인에 유용
    
    bool suffix;       // -s 옵션: 중복 파일명 시 숫자 접미사 추가
                       // true일 때 file.txt가 이미 있으면 file_1.txt로 생성
                       // 파일 손실 없이 안전한 이동을 보장
} mv_options_t;

/* ==================== 함수 선언부 ==================== */

/**
 * 옵션 구조체를 기본값으로 초기화
 * @param opts 초기화할 mv_options_t 구조체 포인터
 * 
 * 모든 불린 필드를 false로 설정하여 mv의 기본 동작 상태로 만든다.
 * 프로그램 시작 시 반드시 호출되어야 하는 초기화 함수이다.
 */
void init_options(mv_options_t *opts);

/**
 * 명령줄 인자를 파싱하여 옵션 구조체에 저장
 * @param argc main()에서 받은 인자 개수
 * @param argv main()에서 받은 인자 배열
 * @param opts 파싱 결과를 저장할 옵션 구조체 포인터
 * @return 성공 시 파일 인자 시작 인덱스, 실패 시 -1
 * 
 * getopt()를 사용하여 POSIX 표준 방식으로 옵션을 파싱한다.
 * 지원 옵션: -i, -f, -n, -v, -s
 * 상호 배타적 옵션들의 충돌도 이 함수에서 해결한다.
 */
int parse_options(int argc, char *argv[], mv_options_t *opts);

/**
 * 파일 덮어쓰기 여부를 옵션에 따라 결정
 * @param dest 덮어쓸 대상 파일 경로
 * @param opts 현재 설정된 옵션들
 * @return 덮어쓰기 허용 시 true, 거부 시 false
 * 
 * 옵션 우선순위: -n > -f > -i > 기본값(허용)
 * 대상 파일이 존재하지 않으면 항상 true 반환
 */
bool should_overwrite(const char *dest, mv_options_t *opts);

/**
 * 중복 파일명 시 고유한 이름 생성 (-s 옵션용)
 * @param dest 원래 대상 파일 경로
 * @return 동적 할당된 고유 파일명 문자열 (호출자가 free 필요)
 * 
 * 파일명_숫자.확장자 형태로 중복되지 않는 이름을 생성한다.
 * 예: file.txt -> file_1.txt, data -> data_1
 * 메모리를 동적 할당하므로 사용 후 반드시 free() 해야 한다.
 */
char* generate_unique_name(const char *dest);

#endif /* MV_OPTIONS_H */

/*
 * 이 헤더 파일의 사용 패턴:
 * 
 * 1. main()에서 mv_options_t 구조체 선언
 * 2. init_options()로 초기화
 * 3. parse_options()로 명령줄 옵션 파싱
 * 4. 파일 이동 시 should_overwrite()로 덮어쓰기 검사
 * 5. -s 옵션 시 generate_unique_name()으로 고유명 생성
 * 
 * 예시:
 * mv_options_t opts;
 * init_options(&opts);
 * int file_start = parse_options(argc, argv, &opts);
 * if (should_overwrite(dest, &opts)) {
 *     // 파일 이동 수행
 * }
 */