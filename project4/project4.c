/* The program below receives a program's code its arguments,input,
expected output and maximum execution time and grades it based on 4
different categories,procuducing the final grade out of 100*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

#define FAIL -1
#define SUCCESS 0  

//Grading data-structure
typedef struct {
    int compilation;
    int timeout ;
    int execution ;
    int memoryAccess ;
    unsigned int finalGrade ;
}grade;

//Function outputs end-results
void print(grade score){
    score.finalGrade = 
    fmax(0,score.compilation + score.execution + score.memoryAccess + score.timeout);
    
    printf("\n");
    printf("Compilation: %d\n",score.compilation);
    printf("\n");
    printf("Termination: %d\n",score.timeout);
    printf("\n");
    printf("Output: %d\n",score.execution);
    printf("\n");
    printf("Memory access: %d\n",score.memoryAccess);
    printf("\n");
    printf("Score: %d\n",score.finalGrade);
}

//Program compilation ,checking for error and warnings
int programCompile(char *file,char *executable,char *errorFile,grade *score){
    int P1 ;
    int errorFd ;
    FILE *errFILE ;
    int status ;
    int numWord = 0;
    char nextChar ;
    char *word ;
    char *arguments[] = {"gcc","-Wall",file,"-o",executable,NULL} ;

    if(!(P1 = fork())){
        errorFd = open(errorFile,O_RDWR | O_CREAT,0444);
        if(errorFd == -1){
            perror("open");
            return FAIL;
        }
        if(dup2(errorFd,STDERR_FILENO) == -1){
            perror("dup2");
            return FAIL;
        }
        execvp("gcc",arguments);
        perror("execvp");
        return FAIL;
    }
    errno = 0;
    waitpid(P1,&status,0);
    if(errno != 0){
        perror("waitpid");
        return FAIL;
    }

    if(status == -1){
        return FAIL ;
    }
    errFILE = fopen(errorFile,"r");
    if(errFILE != NULL){
        while(fscanf(errFILE,"%ms",&word) != EOF){
            numWord ++;
            fscanf(errFILE,"%c",&nextChar);
            if(strcmp(word,"error:") == 0 && numWord == 2){
                score->compilation = -100 ;
                free(word);
                errno = 0;
                fclose(errFILE);
                if(errno != 0){
                    perror("fclose");
                }
                return FAIL;
            }
            else if(strcmp(word,"warning:") == 0){
                numWord = 0 ;
                score->compilation += -5 ;
            }
            if(nextChar == '\n'){
                numWord = 0;
            }
            else{
                errno = 0;
                fseek(errFILE,-1L,SEEK_CUR);
                if(errno != 0){
                    perror("fseek");
                }
            }
            free(word);
        }
        errno = 0;
        fclose(errFILE);
        if(errno != 0){
            perror("fclose");
        }
    }
    else{
        return FAIL ;
    }

    return SUCCESS ;
}

//Function reads arguments from a file and returns them in the appropriate
//form to be used to execute the executable created from the first stage 
char **argumentRead(char *executable,char *args){
    char **arguments ;
    char *argument ;
    char **temp ;
    int size = 1;
    int i ;
    FILE *argsFile ;

    
    arguments = malloc(sizeof(char**));
    if(arguments == NULL){
        return NULL ;
    }
    arguments[0] = executable ;

    argsFile = fopen(args,"r");
    if(argsFile == NULL){
        perror("fopen");
        return NULL;
    }
    while(fscanf(argsFile,"%ms",&argument) != EOF){
        if(argument == NULL){
            return NULL ;
        }
        size++ ;
        temp = realloc(arguments,size * sizeof(char**));
        if(temp == NULL){
            for(i=1; i<size; i++){
                free(arguments[i]);
            }
            free(arguments);
            return NULL ;
        }
        arguments = temp ;
        arguments[size-1] = argument ; 
    }
    size++;
    temp = realloc(arguments,size * sizeof(char**));
    if(temp == NULL){
        for(i=1; i<size; i++){
            free(arguments[i]);
        }
        free(arguments);
        return NULL;
    }
    arguments = temp ;
    arguments[size-1] = NULL ;

    errno = 0;
    fclose(argsFile);
    if(errno != 0){
        perror("fclose");
    }

    return arguments ;
}

//Execution of the program and grading during the execution 
//By running two processes concurrently that communicate via a pipe
int execution(char *executable,char *args,char *input,
                    char *output,int timeout,grade *score){
    pid_t P2 ;
    pid_t P3 ;
    int fd1 ;
    int i ;
    int signum ;
    int pipeFDs[2] ;
    int statusP2 ;
    int statusP3 ;
    sigset_t alarm ;
    struct itimerval time;
    char **arguments ;
    
    arguments = argumentRead(executable,args);
    if(arguments == NULL){
        return FAIL ;
    }

    if(pipe(pipeFDs) == -1){
        free(arguments);
        return FAIL;
    }

    
    fd1 = open(input,O_RDONLY,0444);
    if(fd1 == -1){
        free(arguments);
        return FAIL;
    }

    if(!(P2 = fork())){
        errno = 0;
        if(dup2(fd1,STDIN_FILENO) == -1){
            perror("dup2");
            return FAIL ;
        }
        close(fd1);
        if(dup2(pipeFDs[1],STDOUT_FILENO) == -1){
            perror("dup2");
            return FAIL;
        }
        close(pipeFDs[1]);
        close(pipeFDs[0]);
        if(errno != 0){
            perror("close");
        }
        execv(executable,arguments);
        perror("execv");
        for(i=1; arguments[i] != NULL; i++){
            free(arguments[i]);
        }
        free(arguments);
        return FAIL;
    }
    for(i=1; arguments[i] != NULL; i++){
        free(arguments[i]);
    }
    free(arguments);
    errno = 0;
    close(fd1);
    close(pipeFDs[1]);
    if(errno != 0){
        perror("close");
    }

    if(!(P3 = fork())){
        if(dup2(pipeFDs[0],STDIN_FILENO) == -1){
            perror("dup2");
        }
        errno = 0;
        close(pipeFDs[0]);
        if(errno != 0){
            perror("close");
        }
        execl("./p4diff","p4diff",output,NULL);
        perror("execl");
        return FAIL;
    }
    errno = 0 ;
    close(pipeFDs[0]);
    if(errno != 0){
        perror("close");
    }

    errno = 0;
    sigemptyset(&alarm);
    sigaddset(&alarm,SIGALRM);
    sigaddset(&alarm,SIGCHLD);
    sigprocmask(SIG_BLOCK, &alarm, NULL);
    
    time.it_value.tv_sec = timeout ;
    time.it_value.tv_usec = 0;
    time.it_interval.tv_sec = 0;
    time.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL,&time,NULL);
    if(errno != 0){
        return FAIL;
    }
    sigwait(&alarm,&signum);
    if (signum == SIGALRM) {
        kill(P2,SIGKILL);
        score->timeout = -100 ;
    }

    errno = 0;
    waitpid(P2,&statusP2,0);
    waitpid(P3,&statusP3,0);
    if(errno != 0){
        perror("waitpid");
        return FAIL ;
    }
    if(WIFSIGNALED(statusP2) == 1 && 
        (WTERMSIG(statusP2) == SIGSEGV 
        || WTERMSIG(statusP2) == SIGABRT 
        || WTERMSIG(statusP2) == SIGBUS)){

            score->memoryAccess = -15 ;
    }
    score->execution = WEXITSTATUS(statusP3);

    return SUCCESS ;
}


int main(int argc, char *argv[]){
    char *executable ;
    char *errorFile ;
    grade score = {0};
    int returnValue = 0;
    unsigned int timeout ;

    if(argc != 6){
        return 1 ;
    }
    if(atoi(argv[5]) > 0){
        timeout = atoi(argv[5]) ;
    }
    else{
        return 42 ;
    }
    
    executable = malloc((strlen(argv[1]) - 1) * sizeof(char));
    if(executable != NULL){
        strncpy(executable,argv[1],(strlen(argv[1]) - 2));
        executable[strlen(argv[1]) - 2] = '\0' ;
    }
    else{
        return 42 ;
    }

    errorFile = malloc((strlen(argv[1]) + 5) * sizeof(char));
    if(errorFile != NULL){
        strcpy(errorFile,executable);
        strcpy(errorFile + strlen(executable),".err");
    }
    else{
        free(executable);
        return 42 ;
    }

    returnValue = programCompile(argv[1],executable,errorFile,&score);
    if(returnValue == FAIL){
        free(executable);
        free(errorFile);
        print(score);
        return FAIL;
    }
    returnValue = execution(executable,argv[2],argv[3],argv[4],timeout,&score);
    if(returnValue == FAIL){
        free(executable);
        free(errorFile);
        print(score);
        return  FAIL;
    }
    
    
    free(executable);
    free(errorFile);
    print(score);

    return 0;
}