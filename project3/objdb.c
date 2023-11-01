#include "objdb.h"

//Database opening
int databaseOpen(char *path,int *fd){
    char identifier[3] = "\0" ;

    close(*fd);

    *fd = open(path,O_RDWR,S_IRWXU);
    if(*fd == -1 && errno == ENOENT){
        *fd = open(path,O_RDWR | O_CREAT,S_IRWXU);
        if(*fd == -1){
            return FAIL ;
        }
        else{
            write(*fd,"DB", 2);
            return SUCCESS ;
        }
    }
    else if(*fd != -1){
        if(read(*fd,identifier,2) != -1){
            if(strcmp(identifier,"DB") == 0){
                lseek(*fd,2,SEEK_CUR);
                return SUCCESS ;
            }
            else{
                *fd = -1 ;
                return INVALID_DB;
            }
        }
        else{
            *fd = -1;
            return FAIL;
        }
    }
    return FAIL;
}

//This function searches for objects inside the file
int search(int *fd,char *objName){
    int i = 2 ;
    int j = 0 ;
    int fileSize ;
    char name[256] ;
    int num ; 

    fileSize = lseek(*fd,0,SEEK_END);
    lseek(*fd,2,SEEK_SET);
    while(i < fileSize){
        read(*fd,name + j,1);
        i++ ;
        if(name[j] == '\0'){
            if(strcmp(name,objName) == 0){
                return EXISTING ;
            }
            else{
                errno = 0 ;
                read(*fd,&num,sizeof(int));
                i += sizeof(int) ;
                i += num ;
                lseek(*fd,i,SEEK_SET);
                if(errno != 0){
                    return FAIL ;
                }
            }
            j = 0;
        }
        else{
            j++ ;
        }
    }
    return NON_EXISTING ;
}

//This function improrts new user-given objects
int import(char *path,char *dbSaveName,int *fd){
    int objFD ;
    int bytes ;
    char contents[512] ;
    int bytesInBuffer ;
    int bytesRead = 0;
    int bytesWritten = 0 ;

    if(*fd == -1){
        return MISSING_DB ;
    }

    objFD = open(path,O_RDWR,S_IRWXU);
    if(objFD == -1){
        return FAIL ;
    }

    if(search(fd,dbSaveName) == NON_EXISTING){
        write(*fd,dbSaveName,strlen(dbSaveName));
        write(*fd,"\0",1);

        bytes = lseek(objFD,0,SEEK_END);
        if(bytes != -1){
            errno = 0 ;
            write(*fd,&bytes,sizeof(int));
            lseek(objFD,0,SEEK_SET);
            while(bytesRead != bytes || bytesWritten != bytes){
                if(bytesWritten == bytesRead){
                    bytesInBuffer = read(objFD,contents,512);
                    bytesRead += bytesInBuffer ;
                }
                bytesWritten += write(*fd,contents,bytesInBuffer);
            }
            if(errno != 0){
                return FAIL ;
            }
            return SUCCESS ;
        }
        return FAIL ;
    }else{
        return INVALID_OBJ ;
    }
    

    return 1;
}

//This function searches if the given object name contains the given string
int stringFinder(char *namepart,char *name){
    int i ;
    int z ;

    for(z=0,i=0; z < (strlen(name) + 1); z++){
        if(namepart[i] == '\0'){
            return SUCCESS ;
        }
        else if(namepart[i] == name[z]){
            i++ ;
        }
        else if(namepart[i] != name[z]){
            i = 0;
        }
    }
    return FAIL ;
}

//This function searches for object names that contain the given string 
int find(int *fd,char *namepart,findReturn *findResults){
    int i = 2 ;
    int j = 0;
    int dataSize ;
    int filesize ;
    char name[256];
    char **temp ;
    
    errno = 0 ;
    if(*fd == -1){
        return MISSING_DB ;
    }

    filesize = lseek(*fd,0,SEEK_END);
    lseek(*fd,2,SEEK_SET);
    while(i < filesize){
        read(*fd,name + j,1);
        i++ ;
        if(name[j] == '\0'){
            if(strcmp(namepart,"*") != 0  
               && stringFinder(namepart,name) == SUCCESS){
                (findResults->size)++;
                temp = (char **)realloc(findResults->findNames,
                                    findResults->size * sizeof(char**));
                if(temp != NULL){
                    findResults->findNames = temp ;
                    findResults->findNames[findResults->size - 1] = 
                                (char*)malloc(strlen(name) * sizeof(char) + 1);
                    strcpy(findResults->findNames[findResults->size - 1],name);
                }
                else{
                   (findResults->size)--; 
                }
            }
            else if(strcmp(namepart,"*") == 0){
                (findResults->size)++;
                temp = (char **)realloc(findResults->findNames,
                                    findResults->size * sizeof(char**));
                if(temp != NULL){
                    findResults->findNames = temp ;
                    findResults->findNames[findResults->size - 1] = 
                                (char*)malloc(strlen(name) * sizeof(char) + 1);
                    strcpy(findResults->findNames[findResults->size - 1],name);
                }
                else{
                   (findResults->size)--; 
                }
            }
            read(*fd,&dataSize,sizeof(int));
            i += sizeof(int) ;
            i += dataSize ;
            lseek(*fd,i,SEEK_SET);
            j = 0;
        }
        else{
            j++ ;    
        }
    }
    if(errno != 0){
        return FAIL ;
    }
    return SUCCESS ;
}

//This function exports the given object in a file 
int export(int *fd,char *objName,char *fnName){
    int exportFD ;
    int returnValue ;
    int bytes ;
    int bytesWritten = 0 ;
    int bytesRead = 0 ;
    int bytesInBuffer ;
    char contents[512] ;

    if(*fd == -1){
        return MISSING_DB ;
    }
    exportFD = open(fnName,O_RDWR,S_IRWXU);
    if(exportFD == FAIL && errno == ENOENT){
        exportFD = open(fnName,O_RDWR | O_CREAT,S_IRWXU);
        if(exportFD == FAIL){
            return FILE_OPEN_ERROR ;
        }
    }
    else{
        return FILE_OPEN_ERROR ;
    }

    returnValue = search(fd,objName) ;
    if(returnValue == NON_EXISTING){
        unlink(fnName);
        return INVALID_OBJ ;
    }
    else if(returnValue == FAIL){
        return FAIL ;
    }

    errno = 0 ;
    returnValue = read(*fd,&bytes,sizeof(int));
    if(returnValue != -1){
        while(bytesRead != bytes || bytesWritten != bytes){
            if(bytesWritten == bytesRead && (bytes - bytesRead) >= 512){
                bytesInBuffer = read(*fd,contents,512);
                bytesRead += bytesInBuffer ;
            }
            else if(bytesWritten == bytesRead && (bytes - bytesRead) < 512){
                bytesInBuffer = read(*fd,contents,(bytes - bytesRead));
                bytesRead += bytesInBuffer ;
            }
            bytesWritten += write(exportFD,contents,bytesInBuffer);
        }
    }
    if(errno != 0){
        return FAIL ;
    }
    return SUCCESS ;
}

//This function deletes a given oblect
int delete(int *fd,char *fnName,char *objName){
    int fd2 ;
    int bytesToDelete = 0;
    char contents[512] ;
    int bytes ;
    int pos ;
    int bytesRead = 0;
    int bytesWritten = 0 ;
    int bytesInBuffer ;
    int fileSize ;

    if(*fd == -1){
        return MISSING_DB ;
    }

    fileSize = lseek(*fd,0,SEEK_END);
    if(search(fd,objName) == EXISTING){
        fd2 = open(fnName,O_RDWR,S_IRWXU);
        if(fd2 == -1){
            return FAIL ;
        }

        bytesToDelete = (strlen(objName) + 1);
        pos = lseek(*fd,-bytesToDelete,SEEK_CUR);
        lseek(*fd,bytesToDelete,SEEK_CUR);
        lseek(fd2,pos,SEEK_SET);

        bytesToDelete += read(*fd,&bytes,sizeof(int));
        bytesToDelete += bytes ;
        lseek(*fd,bytes,SEEK_CUR);
        while(bytesRead != (fileSize - bytesToDelete) 
              || bytesWritten != (fileSize - bytesToDelete)){
            if(bytesWritten == bytesRead){
                bytesInBuffer = read(*fd,contents,512);
                if(bytesInBuffer == 0){
                    break ;
                }
                bytesRead += bytesInBuffer ;
            }
            bytesWritten += write(fd2,contents,bytesInBuffer);
        }
        ftruncate(*fd,(fileSize - bytesToDelete));

        return SUCCESS ;
    }
    return INVALID_OBJ ;

}

int fileClose(int *fd){
    if(close(*fd) == -1){
        return MISSING_DB ;
    }
    *fd = -1;
    return SUCCESS ;
}