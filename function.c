#include "header.h"

const int CODE_DIR = 1;
const int CODE_FILE = 2;
const int ERR_CODE = -1;
const int END_OF_FILE = -2; 
const char *FORMAT_FILE = ".arc";
const int MAXNAME = 256;
const int MODE_CRT = 0755;


int archive(char *dir_name, int arch_file, int nesting)
{
    DIR* dir = opendir(dir_name);
    if (dir == NULL)
    {
        printf("Не удалось открыть папку %s\n", dir_name);
        perror("");
        closedir(dir);
        return ERR_CODE;
    }
    
    if (chdir(dir_name) == ERR_CODE)
    {
        printf("Не удалось перейти в директорию <%s>\n", dir_name);
        perror("");
        closedir(dir);
        return ERR_CODE;
    }

    struct dirent *dirs_content = readdir(dir);
    struct stat stat_dirs_content;
      
    //Для каждого файла записать в архив
    while(dirs_content != NULL)
    {
        if((strncmp((*dirs_content).d_name, ".", 1) && strncmp((*dirs_content).d_name, "..", 2)) != 0)
        {
            lstat((*dirs_content).d_name, &stat_dirs_content);
            if (S_ISDIR(stat_dirs_content.st_mode))
            {
                archive_dir((*dirs_content).d_name, arch_file, nesting + 1);
                archive((*dirs_content).d_name, arch_file, nesting + 1);
            }
            else
            {
                archive_file((*dirs_content).d_name, arch_file, nesting + 1);
            }
        
        }
        dirs_content = readdir(dir);
    }

    if (chdir("..") == ERR_CODE)
    {
        printf("Не удалось вернуться из директорию <%s>\n", dir_name);
        perror("");
        closedir(dir);
        return ERR_CODE;
    }

    closedir(dir);
    return 0;
}
// архив
    // если код == CODE_DIR
    // <уровень вложенности><код><размер имени><имя папки>
    // если код == CODE_FILE
    // <уровень вложенности><код><размер имени><имя файла><размер файла><информация>  

int unarchive(char *archiver_name, char *dir_name)
{
    //проверка формата файла
    if (strncmp(archiver_name + strlen(archiver_name) - strlen(FORMAT_FILE), FORMAT_FILE, strlen(FORMAT_FILE)) != 0)
    {
        printf("Файл %s не является архивом с форматом %s\n", archiver_name, FORMAT_FILE);
        return ERR_CODE;
    }
  
    int arch_file = open(archiver_name, O_RDONLY);
    if (arch_file < 0) 
    {
        printf("Не удалось открыть файл %s\n", archiver_name);
        perror("");
        return ERR_CODE;
    }
    //Создание папки
    if (mkdir(dir_name, MODE_CRT) == ERR_CODE)
    {
        printf("Не удалось создать директорию <%s>\n", dir_name);
        perror("");
        close(arch_file);
        return ERR_CODE;
    }

    DIR *dir = opendir(dir_name);
    
    if (chdir(dir_name) == ERR_CODE)
    {
        printf("Не удалось перейти в директорию <%s>\n", dir_name);
        perror("");
        close(arch_file);
        closedir(dir);
        return ERR_CODE;
    }
    
    int prev_nesting = 1;
    int cur_nesting = 0;
    int type = 0;
    int size_name = 0;
    char prev_name[MAXNAME];
    
    while (1)
    {
        //чтение из архива уровня вложенности
        int read_code = read(arch_file, &cur_nesting, sizeof(int));
        if (read_code == ERR_CODE) 
        {
            printf("Ошибка чтения из файла\n");
            perror("");
            closedir(dir);
            close(arch_file);
            return ERR_CODE;
        }
        else if (read_code == 0) 
        {
            close(arch_file);
            closedir(dir);
            return END_OF_FILE;
        }

        //чтение из архива типа (файл или папка)
        if (read(arch_file, &type, sizeof(int)) == ERR_CODE) 
        {
            printf("Ошибка чтения из файла\n");
            perror("");
            closedir(dir);
            close(arch_file);
            return ERR_CODE;
        }

        //чтение из архива размера имени
        if (read(arch_file, &size_name, sizeof(int)) == ERR_CODE) 
        {
            printf("Ошибка чтения из файла\n");
            perror("");
            closedir(dir);
            close(arch_file);
            return ERR_CODE;
        }

        char *name = (char*)malloc(size_name + 1);
        if (name == NULL) 
        {
            printf("Ошибка выделения памяти\n");
            perror("");
            closedir(dir);
            close(arch_file);
            return ERR_CODE;
        }

        //чтение из архива имени
        if (read(arch_file, name, size_name) == ERR_CODE) 
        {
            printf("Ошибка чтения из файла\n");
            perror("");
            closedir(dir);
            close(arch_file);
            return ERR_CODE;
        }
       
        name[size_name] = '\0'; 
        
        if (prev_nesting == cur_nesting) // если уровень вложенности не поменялся, то остаемся в текущей директории
        {
            if (create_content(name, type, arch_file) == ERR_CODE) 
            {
                closedir(dir);
                close(arch_file);
                return ERR_CODE;
            }
        }
        else if (prev_nesting + 1 == cur_nesting) // если уровень вложенности повысился, то переходим в последнюю созданную директорию
        {
            chdir(prev_name);
            if (create_content(name, type, arch_file) == ERR_CODE) 
            {
                closedir(dir);
                close(arch_file);
                return ERR_CODE;
            }
        }
        else if (prev_nesting > cur_nesting) // если уровень вложенности понизился, то перемещаемся вверх на prev_nesting - cur_nesting уровней
        {
            for (int i = 0; i < prev_nesting - cur_nesting; i++)
                chdir("..");
            
            if (create_content(name, type, arch_file) == ERR_CODE) 
            {
                closedir(dir);
                close(arch_file);
                return ERR_CODE;
            }
        }
        strcpy(prev_name, name);
        free(name);
        prev_nesting = cur_nesting;
    }
}

int create_content(char *name, int type, int arch_file)
{
    if (type == CODE_DIR) //если папка
    {
        if (mkdir(name, MODE_CRT) == ERR_CODE) //создать папку
        {
            printf("Не удалось создать директорию <%s>\n", name);
            perror("");
            return ERR_CODE;
        }
    }
    else if (type == CODE_FILE)
    {
        int unarch_file = open(name, O_CREAT|O_RDWR, MODE_CRT); //создать файл
        if (unarch_file < 0) 
        {
            printf("Не удалось открыть файл %s\n", name);
            perror("");
            return ERR_CODE;
        }

        off_t size_file;
        if (read(arch_file, &size_file, sizeof(off_t)) == ERR_CODE) 
        {
            printf("Ошибка чтения из файла\n");
            perror("");
            close(unarch_file);
            return ERR_CODE;
        }
        char tmp;
        int i = size_file;
        //записать информацию файл из архива
        while(i > 0)
        {  
            if (read(arch_file, &tmp, sizeof(char)) == ERR_CODE) 
            {
                printf("Ошибка чтения из файла\n");
                perror("");
                close(unarch_file);
                return ERR_CODE;
            }
            write(unarch_file, &tmp, sizeof(char));
            i--;
        }
        close(unarch_file);
    }
    return 0;
}

int archive_file(char *file_name, int arch_file, int nesting)
{
    int source_file = open(file_name, O_RDONLY);
    int size_name_file = strlen(file_name);
    if (source_file < 0) 
    {
        printf("Не удалось открыть файл %s\n", file_name);
        perror("");
        return ERR_CODE;
    }
    if (size_name_file > MAXNAME) 
    {
        printf("Размер имени файла превышает %i\n", MAXNAME);
        perror("");
        close(source_file);
        return ERR_CODE;
    }

    // запись уровня вложенности
    write(arch_file, &nesting, sizeof(int));
    
    struct stat file_info;
    lstat(file_name, &file_info);
    off_t file_size = file_info.st_size;
    
    //запись кода, что это файл
    write(arch_file, &CODE_FILE, sizeof(int));

    char tmp;
    int read_code = read(source_file, &tmp, sizeof(char));
    if(read_code == ERR_CODE) 
    {
        printf("Ошибка чтения из файла %s\n", file_name);
        perror("");
        close(source_file);
        return ERR_CODE;
    }

    //запись размера имени
    write(arch_file, &size_name_file, sizeof(int)); 
   
    //запись имени
    write(arch_file, file_name, size_name_file);
     
    //запись размера файла
    write(arch_file, &file_size, sizeof(off_t));
   
    //запись информации из файла
    while(read_code > 0) 
    {
        write(arch_file, &tmp, sizeof(char));
        read_code = read(source_file, &tmp, sizeof(char)); 
    }

    close(source_file);
    return 0;
}

int archive_dir(char *dir_name, int arch_file, int nesting)
{
    write(arch_file, &nesting, sizeof(int));
    int size_dir_name = strlen(dir_name);
     
    write(arch_file, &CODE_DIR, sizeof(int));
    //запись размера имени
    write(arch_file, &size_dir_name, sizeof(int)); 
   
    //запись имени
    write(arch_file, dir_name, size_dir_name);
}

void format_archive(char *archiver_name)
{
    strcat(archiver_name, FORMAT_FILE);
}
