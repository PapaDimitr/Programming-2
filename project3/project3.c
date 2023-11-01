/* 
This programm with the use of the objdb functions creates a virual
database inside a file and has the ability to manage it by deleting
,inserting or exporting digital objects.
*/
#include "objdb.h"

int main(int argc ,char *argv[]){
    int databaseFd = -1;
    int returnValue ;
    char command ;
    int i ;
    char *dbName = NULL;
    char *fnName = NULL ;
    char objName[256] = "\0" ;
    char namepart[256] = "\0" ;
    findReturn findResults = {0,NULL} ;

    while(1){
        scanf(" %c", &command);
        switch(command){
            case 'o':{
                if(dbName != NULL){
                    free(dbName);
                }
                scanf(" %ms", &dbName);
                returnValue = databaseOpen(dbName,&databaseFd);
                if(returnValue == FAIL){
                    fprintf(stderr,"\nError opening %s.\n",dbName);
                }
                else if(returnValue == INVALID_DB){
                    fprintf(stderr,"\nInvalid db file %s.\n",dbName);
                }
                break;
            }
            case 'i':{
                scanf(" %ms %s", &fnName, objName);
                returnValue = import(fnName,objName,&databaseFd);
                if(returnValue == MISSING_DB){
                    fprintf(stderr,"\nNo open db file.\n");   
                }
                else if(returnValue == FAIL && errno == ENOENT){
                    fprintf(stderr,"\nFile %s not found.\n",fnName);
                }
                else if(returnValue == INVALID_OBJ){
                    fprintf(stderr, "\nObject %s already in db.\n",objName);
                }
                else if(returnValue == FAIL){
                    perror("\nUnexpected Error");
                    return 42 ;
                }
                free(fnName);
                break ;
            }
            case 'f':{
                scanf(" %s", namepart);
                returnValue = find(&databaseFd,namepart,&findResults) ;
                printf("\n##\n");
                if(findResults.findNames != NULL){
                    for(i=0; i < findResults.size; i++){
                        printf("%s\n", findResults.findNames[i]);
                        free(findResults.findNames[i]);
                    }
                    findResults.size = 0 ;
                    free(findResults.findNames);
                    findResults.findNames = NULL ;
                }
                else if(returnValue == MISSING_DB){
                    fprintf(stderr,"\nNo open db file.\n");   
                }
                else if(returnValue == FAIL){
                    perror("\nUnexpected Error");
                    return 42 ;
                }
                break ;
            }
            case 'e':{
                scanf(" %s %ms", objName, &fnName);
                returnValue = export(&databaseFd,objName,fnName);
                if(returnValue == MISSING_DB){
                    fprintf(stderr,"\nNo open db file.\n");
                }
                else if(returnValue == FILE_OPEN_ERROR){
                    fprintf(stderr,"\nCannot open file %s.\n",fnName);
                }
                else if(returnValue == INVALID_OBJ){
                    fprintf(stderr,"\nObject %s not in db.\n",objName);   
                }
                else if(returnValue == FAIL){
                    perror("\nUnexpected Error");
                    return 42 ;
                }
                free(fnName);
                break ;
            }
            case 'd':{
                scanf(" %s", objName);
                returnValue = delete(&databaseFd,dbName,objName);
                if(returnValue == MISSING_DB){
                    fprintf(stderr,"\nNo open db file.\n");
                }
                else if(returnValue == INVALID_OBJ){
                    fprintf(stderr,"\nObject %s not in db.\n",objName);   
                }
                else if(returnValue == FAIL){
                    perror("\nUnexpected Error");
                    return 42 ;
                }
                break ;
            }
            case 'c':{
                returnValue = fileClose(&databaseFd);
                free(dbName);
                dbName = NULL ;
                if(returnValue == MISSING_DB){
                    fprintf(stderr,"\nNo open db file.\n");
                }else if(returnValue == FAIL){
                    perror("\nUnexpected Error");
                    return 42 ;
                }
                break ;
            }
            case 'q':{
                free(dbName);
                return 0;
            }
            default :{
                fprintf(stderr,"\nInvalid command.\n");
                while(getchar() != '\n'){
                    continue;
                }

            }
        }
    }
    return 0;
}
