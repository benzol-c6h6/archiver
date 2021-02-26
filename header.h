#pragma once
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern const int CODE_DIR;
extern const int CODE_FILE;
extern const int END_OF_FILE;
extern const int ERR_CODE;
extern const char *FORMAT_FILE;
extern const int MAXNAME;
extern const int MAXSIZEBUF;
extern const int MODE_CRT;

int archive(char *dir_name, int arch_file, int nesting);
int archive_file(char *file_name, int arch_file, int nesting);
int archive_dir(char *dir_name, int arch_file, int nesting);

int unarchive(char *archiver_name, char *dir_name);
int create_content(char *name, int type, int arch_file);

void format_archive(char *archiver_name);
