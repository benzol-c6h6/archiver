#include "header.h"

int main(int argc, char *argv[])
{
    /*argv[1] - режим работы:   <arch> - архивировать
                                <unarch> - разархивировать
    argv[2] - название папки//архива 
    argv[3] - название архива*/
    if (argc == 4)
    {
        if (strncmp(argv[1], "arch", strlen(argv[1])) == 0)
        {
            format_archive(argv[3]);
            printf("\t\tАрхивирование папки %s в архив %s\n", argv[2], argv[3]);
            int arch_file = open(argv[3], O_CREAT|O_RDWR, MODE_CRT);
            if (arch_file < 0)
            {
                printf("Не удалось открыть файл %s\n", argv[3]);
                perror("");
                return ERR_CODE;
            }
            
            archive(argv[2], arch_file, 0);
            close(arch_file);
        }
        else if (strncmp(argv[1], "unarch", strlen(argv[1])) == 0)
        {
            printf("\t\tРазархивирование %s в папку %s\n", argv[2], argv[3]);
            unarchive(argv[2], argv[3]);
        }
        else 
        {
             printf("Ошибка! Неверно введены аргументы:"
            "\n1. Режим работы архиватора: <arch> или <unarch>.\n");
        }
    }
    else 
    {
        printf("Ошибка! Неверно введены аргументы:"
        "\n1. Режим работы архиватора."
        "\n2. Название папки для <arch> и архива для <unarch>."
        "\n3. Название архива для <arch> и название папки для <unarch>.\n");
    }
    return 0;
}