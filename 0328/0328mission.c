#include <stdio.h>

int main()
{
    char c;

    while(1)
    {
        printf("문자 입력 : ");
        scanf(" %c",&c);
        getchar();
        if(c == '0'){
            break;
        }

        if(c >= 'A' &&c<='Z')
            printf("%c의 소문자는 %c입니다.\n",c,c+32);
        if(c>='a' && c<='z')
        printf("%c의 대문자는 %c입니다.\n",c,c-32);
    }

    return 0;
}