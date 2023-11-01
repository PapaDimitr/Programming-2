/* ONOMATEPWNYMO: PAPADIMITRIOU DIMITRIS
   AEM: 03750
 */

/* This programm reads student names' as input and has the 
ability to sort them in ascending order (based of their AEM) 
and can also find a student in the database,modify a student's failed courses 
and even remove a student upon user request */

#include"project1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum status {FAIL = -1,SUCCESS = 1};

/* Data type for a student's fullname*/
typedef struct{
  char firstName[NAMESIZE] ;
  char lastName[NAMESIZE] ;
}NameInfo;

/* Data type with basic student info*/
typedef struct{
  unsigned long int AEM ;
  NameInfo Fullname ;
  unsigned short int FailedCS ; 
}StudentInfo;

/* Array struct with database array and its relevant info*/
typedef struct{
  StudentInfo **mainArray ;
  int size ;
  int sizeIncrease ;
  int sortStatus ;
  int numStudents ;
}Array;


/*This function rewrites a word with uppercase letters*/
void capital(char *word) {
  while (*word) {
    *word = toupper(*word);
    word++;
  }
}


int binarySearch(int size,long unsigned int target,StudentInfo **arr,
                                          unsigned long int *comparisons){

  int start, end, middle ;

  start = 0;
  end = size - 1;

  while(start <= end){
    middle = (start+end)/2;

    if(arr[middle]->AEM > target){
      (*comparisons) += 2;
      end = middle - 1;
    }else if(arr[middle]->AEM < target){
      (*comparisons) += 2;
      start = middle + 1;
    }else{
      (*comparisons) ++;
      return middle ;
    }
  }
  return FAIL;
}

int linearSearch(int size,long unsigned int target,StudentInfo **arr,
                                           unsigned long int *comparisons){
  int i ;

  for(i=0; i<size; i++){
    (*comparisons)++;
    if(arr[i]->AEM == target){
      return i ;
    }
  }
  return FAIL;
}

/* The "find" function */
int Search(Array registrations,unsigned int target,unsigned long int *comparisons){
  if(registrations.sortStatus == 1){
   return binarySearch(registrations.numStudents,
                       target,registrations.mainArray,comparisons);
  }else if(registrations.sortStatus == 0){
   return linearSearch(registrations.numStudents,
                       target,registrations.mainArray,comparisons);
  }
  return -2;
}

/*Student addition function*/
int studentAddition(Array *registrations,StudentInfo input,
                                unsigned long int *comparisons){

  StudentInfo **temp ;
  int a ;
  int pos = 0;

  capital(input.Fullname.firstName);
  capital(input.Fullname.lastName);

  pos = Search(*registrations,input.AEM,comparisons);

  if(pos != FAIL){
    return FAIL;
  }
  if((registrations->numStudents - 1) >= 0 &&
      input.AEM < registrations->mainArray[registrations->numStudents - 1]->AEM){
    registrations->sortStatus = 0;
  }

  if(registrations->numStudents != registrations->size){
    registrations->mainArray[registrations->numStudents] = 
                                (StudentInfo *)malloc(sizeof(StudentInfo));

    if(registrations->mainArray[registrations->numStudents] != NULL){
      *registrations->mainArray[registrations->numStudents] = input ;
      registrations->numStudents++;
      return SUCCESS;
    }
    else{
      return FAIL;
    }

  }else if(registrations->numStudents == registrations->size 
           && (registrations->size != 0 || registrations->numStudents !=0)){

    registrations->size += registrations->sizeIncrease ;
    temp = (StudentInfo **)realloc(registrations->mainArray,
                            registrations->size * sizeof(StudentInfo));

    if(temp != NULL){
      registrations->mainArray = temp;
      registrations->mainArray[registrations->numStudents] = 
                                  (StudentInfo *)malloc(sizeof(StudentInfo));

      *registrations->mainArray[registrations->numStudents] = input;
      registrations->numStudents++;

      /*Initialization of extended-arrays' vacant positions*/
      for(a=registrations->numStudents; a<registrations->size; a++){
        registrations->mainArray[a] = NULL ;
      }
      return SUCCESS;
    }else{
      return FAIL;
    }
  }else{ // If array is zero in size (non-existing/cleared).
    registrations->mainArray = 
       (StudentInfo**)malloc(registrations->sizeIncrease * sizeof(StudentInfo));
      
    if(registrations->mainArray != NULL){
      registrations->mainArray[registrations->numStudents] = 
                                  (StudentInfo*)malloc(sizeof(StudentInfo));

      if(registrations->mainArray[registrations->numStudents] != NULL){
        *registrations->mainArray[registrations->numStudents] = input ;
        registrations->numStudents++;
        registrations->size = registrations->sizeIncrease;
        for(a=registrations->numStudents; a<registrations->size; a++){
          registrations->mainArray[a] = NULL ;
        }
        return SUCCESS ;
      }
    }else{
      return FAIL ;
    }
  }
  return -2;
}

/* The function modifies a student's failed courses */
int mod(Array *registrations,StudentInfo input,unsigned long int *comparisons){
  int r = Search(*registrations,input.AEM,comparisons);

  if(r == FAIL){
    return FAIL;
  }
  else{
    registrations->mainArray[r]->FailedCS = input.FailedCS ;
    return SUCCESS;
  }
}

/* Student removal function*/
int studentRemove(Array *registrations,unsigned long int AEM,
                              unsigned long int *comparisons){

  int r;
  StudentInfo **temp;

  r = Search(*registrations,AEM,comparisons) ;

  if(r != FAIL){
    free(registrations->mainArray[r]);
    if(r == registrations->numStudents - 1){
      registrations->mainArray[r] = NULL ;
      registrations->numStudents --;
    }else{
      registrations->mainArray[r] = 
                      registrations->mainArray[registrations->numStudents - 1] ;

      registrations->mainArray[registrations->numStudents - 1] = NULL ;
      registrations->numStudents --;
    }

    /* Size reduction */
    if(registrations->size - registrations->numStudents >= registrations->sizeIncrease){
      temp = (StudentInfo **)realloc(registrations->mainArray,
                (registrations->size - registrations->sizeIncrease) * sizeof(StudentInfo));

      if(temp == NULL){
        return FAIL;
      }
      registrations->mainArray = temp ;
      registrations->size -= registrations->sizeIncrease;
    }
    registrations->sortStatus = 0;
    return SUCCESS;
  }
  else{
    return FAIL ;
  }

}

void print(Array *registrations){
  int i ;

  printf("\n##\n");
  for(i=0; i<registrations->numStudents; i++){
    printf("%lu",registrations->mainArray[i]->AEM);
    printf(" %s",registrations->mainArray[i]->Fullname.firstName);
    printf(" %s",registrations->mainArray[i]->Fullname.lastName);
    printf(" %d\n",registrations->mainArray[i]->FailedCS);
  }
}

void clear(Array *registrations){
  int i ;

  if(registrations->mainArray != NULL){
    for(i=0; i<registrations->numStudents; i++){
      free(registrations->mainArray[i]);
      registrations->mainArray[i] = NULL;
    }
    free(registrations->mainArray);
    registrations->mainArray = NULL;
    registrations->numStudents = 0 ;
    registrations->size = 0;
  }
}

/*Insertion sort function*/
void sort(Array *registrations,unsigned long int *comparisons){
    int i, j ; 
    StudentInfo *temp ;
    unsigned long int tempAEM ;

  for(i=1; i<registrations->numStudents; i++){
      temp = registrations->mainArray[i] ;
      tempAEM = registrations->mainArray[i]->AEM ;
      j = i;
      
    while((j > 0) && registrations->mainArray[j-1]->AEM > tempAEM){
      registrations->mainArray[j] = registrations->mainArray[j-1];
      j-- ;
      registrations->mainArray[j] = temp ;
      (*comparisons)++ ;
    }
    if(j>0){
      (*comparisons)++ ;
    }
  }
  registrations->sortStatus = 1;
}

int main(int argc, char *argv[]){
  Array registrations ; /* Main database */

  /* Struct used to "receive" any student related input*/              
  StudentInfo input = {0,{{'\0'},{'\0'}},0} ;

  /*Temporary pointers for string input*/
  char *tempStr1,*tempStr2 ;
  int k ;
  unsigned long int comparisons = 0;
  char command ;   

  if(argc != 3){
    return 42;
  }

  //Allocation of the initial array
  registrations.mainArray = 
                  (StudentInfo **)malloc(atoi(argv[1]) * sizeof(StudentInfo));

  for(k=0; k<atoi(argv[1]); k++){
    registrations.mainArray[k] = NULL ;
  }
  if(registrations.mainArray == NULL){
    return 43;
  }

  /*Setting up database main info*/
  registrations.numStudents = 0;
  registrations.size = atoi(argv[1]);
  registrations.sizeIncrease = atoi(argv[2]);
  registrations.sortStatus = 1;


  while(1){
    scanf(" %c",&command);
    switch(command){
      case 'a':{
        scanf(" %lu %ms %ms %hu",&input.AEM,&tempStr1,&tempStr2,&input.FailedCS);
        if(strlen(tempStr1) >= NAMESIZE || strlen(tempStr2) >= NAMESIZE){
          printf("\nTrimmed name(s).\n");
          if(strlen(tempStr1) >= NAMESIZE){
            strncpy(input.Fullname.firstName,tempStr1,NAMESIZE-1);
          }else{
            strcpy(input.Fullname.firstName,tempStr1);
          }
          if(strlen(tempStr2) >= NAMESIZE){
            strncpy(input.Fullname.lastName,tempStr2,NAMESIZE-1);
          }else{
            strcpy(input.Fullname.lastName,tempStr2);
          }
        }else{
          strcpy(input.Fullname.firstName,tempStr1);
          strcpy(input.Fullname.lastName,tempStr2);
        }
        free(tempStr1);
        tempStr1 = NULL;
        free(tempStr2);
        tempStr2 = NULL ;

        if(input.AEM == 0 || input.FailedCS == 0){
          printf("\nInvalid input.\n");
        }else{
          if(studentAddition(&registrations,input,&comparisons) == SUCCESS){
            printf("\nA-OK %lu,",input.AEM);
            printf(" %d %d\n",registrations.numStudents,registrations.size);
          }else{
            printf("\nA-NOK %lu,",input.AEM);
            printf(" %d %d\n",registrations.numStudents,registrations.size);
          }
        }
        break;
      }
      case 'm':{
        scanf(" %lu %hu",&input.AEM,&input.FailedCS);
        if(input.FailedCS == 0 || input.AEM == 0){
          printf("\nInvalid input.\n");
        }else{
          if(mod(&registrations,input,&comparisons) == SUCCESS){
            printf("\nM-OK %lu\n",input.AEM);
          }else{
            printf("\nM-NOK %lu\n",input.AEM); 
          }
        }
        break;
      }
      case 's':{
        sort(&registrations,&comparisons);
        printf("\nS-OK\n");
        fprintf(stderr,"\n$%lu\n",comparisons);
        break;
      }
      case 'r':{
        scanf(" %lu", &input.AEM);
        if(studentRemove(&registrations,input.AEM,&comparisons) == SUCCESS){
          printf("\nR-OK %lu, %d",input.AEM,registrations.numStudents);
          printf(" %d\n",registrations.size);
        }else{
          printf("\nR-NOK %lu, %d",input.AEM,registrations.numStudents);
          printf(" %d\n",registrations.size);
        }
        break;
      }
      case 'f':{
        scanf(" %lu",&input.AEM);
        k = Search(registrations,input.AEM,&comparisons);
        if(k != FAIL){
          printf("\nF-OK %s",registrations.mainArray[k]->Fullname.firstName);
          printf(" %s",registrations.mainArray[k]->Fullname.lastName);
          printf(" %d\n",registrations.mainArray[k]->FailedCS);
        }else{
          printf("\nF-NOK %lu\n",input.AEM);
        }
        fprintf(stderr,"\n$%lu\n",comparisons);
        break;
      }
      case 'p':{
        print(&registrations);
        break;
      }
      case 'c':{
        clear(&registrations);
        printf("\nC-OK\n");
        break;
      }
      case 'q':{
        clear(&registrations);
        return 0;
      }   
    }
    comparisons = 0; 
  }
}

