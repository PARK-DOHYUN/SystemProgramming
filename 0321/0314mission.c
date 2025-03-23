#include <stdio.h>

int main()
{
    char sn[11];
    char name[11];

    printf("학번을 입력하세요 : ");
    scanf("%s", sn);

    getchar();

    printf("이름을 입력하세요 : ");
    scanf("%s", name);

    printf("학번 : %s\n",sn);
    printf("이름 : %s\n",name);

    return 0;
}
