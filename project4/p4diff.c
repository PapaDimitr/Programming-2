/* The below program is being executed during the execution
phase of the main program(project4.c) receiving output from the
other process via pipe and comparing its output with the expected-one*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <math.h>

//The below function is used to safely read from a pipe using read()
//system call function
int safeRead(int fd,char **contents,int bytes){
    int bytesRead = 0;
    int readReturn ;

    while(bytesRead < bytes){
        readReturn = read(fd,*contents + bytesRead,bytes - bytesRead);
        if(readReturn == -1){
            perror("read");
            exit(-1) ;
        }
        if(readReturn == 0){
            return bytesRead;
        }
        bytesRead += readReturn ;
    }
    return bytesRead ;
}

//This function receives two buffers and the bytes it needs to
//compare and returns the amount of similar bytes 
int compare(char *contents1,char *contents2,int bytes){
    int i ;
    int sameBytes = 0;

    for(i=0; i<bytes; i++){
        sameBytes += (contents1[i] == contents2[i]);
    }
    return sameBytes ;
}

int main(int argc,char *argv[]){
    char *output ;
    char *contentsOUT ;
    char *contentsPIPE ;
    int sameBytes = 0;
    int fileSize = 0;
    int fdOutput ;
    int bytesFile ;
    int bytesPipe ;
    int outputBytes ;

    output = argv[1] ;

    if((fdOutput = open(output,O_RDONLY,0444)) == -1){
        return -1;
    }
    fileSize = lseek(fdOutput,0,SEEK_END);
    if(lseek(fdOutput,0,SEEK_SET) == -1){
        return -1;
    }
    
    contentsOUT = malloc(64 * sizeof(char*));
    contentsPIPE = malloc(64 * sizeof(char*));
    if(contentsOUT == NULL || contentsPIPE == NULL){
        return -1;
    }
    for(outputBytes = 0;  bytesFile != 0 || bytesPipe != 0;){
        bytesPipe = safeRead(STDIN_FILENO,&contentsPIPE,64);
        outputBytes += bytesPipe ;
        bytesFile = safeRead(fdOutput,&contentsOUT,64);
        sameBytes += compare(contentsOUT,contentsPIPE,fmin(bytesPipe,bytesFile));
    }

    free(contentsPIPE);
    free(contentsOUT);

    if(fileSize != 0 || outputBytes != 0){
        return (sameBytes * 100) / fmax(fileSize,outputBytes);
    }

    return 100  ;
}