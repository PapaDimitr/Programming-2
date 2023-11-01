#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define MISSING_DB 4
#define NON_EXISTING 9
#define EXISTING 8
#define INVALID_DB 23
#define INVALID_OBJ 24
#define FILE_OPEN_ERROR 25

enum status {SUCCESS = 0, FAIL = -1} ;

/*
This struct is used to pass find-funtion's 
output outside the function to the main
*/
typedef struct{
    int size ;
    char **findNames ;
}findReturn;

int databaseOpen(char *path,int *fd);
int import(char *path,char *dbSaveName,int *fd);
int fileClose(int *fd);
int search(int *fd,char *objName);
int find(int *fd,char *namepart,findReturn *find);
int export(int *fd,char *objName,char *fnName);
int delete(int *fd,char *fnName,char *objName);
