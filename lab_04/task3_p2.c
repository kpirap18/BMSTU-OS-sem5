#include <stdio.h>
#include <string.h>

#define N 16 

void revstr(register char * str){
    register int i, len = 0;
    //Определяем длину строки
    len = strlen(str);
    //Выполняем реверс символов в строке
    for (i = 0; i <= len / 2; i++){
        *(str + len - i) = *(str + i);  
        *(str + i) = *(str + len - i - 1);
    } 

    //Сдвигаем вторую половину символов влево на 1 символ
    for (i = len / 2; i <= len; i++)
        *(str + i) = *(str + i + 1);
        
    //Устанавливаем символ завершения строки
    *(str + len) = '\0';
}


void read_string(char *str)
{
    printf("Input string (max len = 16): ");
    if (scanf("%s", str) != 1)
    {
        printf("Error, input only string (max len = 16)\n");
    }
}


void main()
{
    // char str[] = "BMSTU IU7-52";
    char str[N];
    
    printf("\n !proc 2 (reverse str) START\n");
    read_string(str);
    printf("String before reverse: %s\n", str);

    revstr(str);

    printf("String after reverse: %s", str);
    printf("\n !proc 2 (reverse str) END\n\n");
}
