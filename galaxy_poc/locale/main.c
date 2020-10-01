#include <locale.h>
#include <stdio.h>

// gcc main.c -o testlocale

int main()
{
        char* lLocal=setlocale(LC_ALL, NULL);

        printf("LC_CTYPE=%s\n", setlocale(LC_CTYPE, NULL));
        printf("LC_ALL:%s\n", lLocal);
        printf("LC_NUMERIC=%s\n", setlocale(LC_NUMERIC, NULL));

        setlocale (LC_ALL,"");
        printf("LC_ALL=%s\n", setlocale(LC_ALL, NULL));

        setlocale(LC_ALL, "C");
        printf("LC_ALL=%s\n", setlocale(LC_ALL, NULL) );
        return 0;
}

