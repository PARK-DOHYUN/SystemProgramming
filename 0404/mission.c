#include <stdio.h>

int main() {
    int num;          // 입력받을 정수
    int count = 0;    // 1의 개수를 저장할 변수
    
    // 0~255 사이의 정수 입력 받기 (입력 검증)
    do {
        printf("0~255 사이의 정수를 입력하세요: ");
        scanf("%d", &num);
        
        if(num < 0 || num > 255) {
            printf("입력 값이 범위를 벗어났습니다. 다시 입력해주세요.\n");
        }
    } while(num < 0 || num > 255);
    
    // 입력된 정수 출력
    printf("입력된 정수: %d\n", num);
    
    // 2진수 표현을 저장할 배열
    int binary[8];
    
    // 정수를 2진수로 변환 (최상위 비트부터)
    for(int i = 7; i >= 0; i--) {
        binary[7-i] = (num >> i) & 1;  // i번째 비트 추출
        if(binary[7-i] == 1) {
            count++;  // 1의 개수 카운트
        }
    }
    
    // 2진수 표현 출력
    printf("2진수 표현: ");
    for(int i = 0; i < 8; i++) {
        printf("%d", binary[i]);
    }
    printf("\n");
    
    // 1의 개수 출력
    printf("1의 개수: %d\n", count);
    
    // 상위 4비트 출력 (최상위 4비트)
    printf("상위 4비트: ");
    for(int i = 0; i < 4; i++) {
        printf("%d", binary[i]);
    }
    printf("\n");
    
    return 0;
}
