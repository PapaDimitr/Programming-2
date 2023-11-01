/* ONOMATEPWNYMO: PAPADIMITRIOU DIMITRIS
   AEM : 03750  
*/

/* The following programm is an updated version of "project1" it has all 
the previous functionalities while this time students can now be assigned
courses and be searched via their name with the assistance of a hash table
*/

#include "project2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum status {FAIL = -1,SUCCESS = 1};
//F stands for FAILURE
enum statusCourseFunctions {F_AEM = 4, F_ID = 5, F_MEM = 6};

/* Data type for a student's fullname */
typedef struct{
  char firstName[NAMESIZE] ;
  char lastName[NAMESIZE];
}NameInfo;

// Data type for courses inside the course list
typedef struct Course{
  unsigned long int ID ;
  struct Course *nextCourse ; 
}Course;

/* Data type with basic student info */
typedef struct StudentInfo{
  unsigned long int AEM ;
  NameInfo Fullname ;
  unsigned short int FailedCS ; 
  Course *CoursesList;
  struct StudentInfo *next ;
  struct StudentInfo *prev ;
}StudentInfo;

// Data type for buckets inside hashArray
typedef struct {
  int bucketSize ;
  StudentInfo *sentinel ;
}Bucket;

typedef struct{
  int hashSize ;
  double numHashes ;
  double load ;
  int initialSize ;
  int largestBucket ;
  Bucket *bucketArray ;
}hashTable;

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

int binarySearch(int size,long unsigned int target,
                StudentInfo **arr,unsigned long int *comparisons){
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

int linearSearch(int size,long unsigned int target,
                 StudentInfo **arr,unsigned long int *comparisons){
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
   return binarySearch(registrations.numStudents,target,registrations.mainArray,comparisons);
  }else if(registrations.sortStatus == 0){
   return linearSearch(registrations.numStudents,target,registrations.mainArray,comparisons);
  }
  return -2;
}

/*Student addition function*/
int hashAddition(StudentInfo *arrayInput,hashTable *hashArray);
int reHashing(Array *registrations,hashTable *hashArray);

int studentAddition(Array *registrations,StudentInfo input,
                    hashTable *hashArray,unsigned long int *comparisons){
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
    //List head initialization
    registrations->mainArray[registrations->numStudents]->CoursesList = NULL ;

    if(registrations->mainArray[registrations->numStudents] != NULL){
      *registrations->mainArray[registrations->numStudents] = input ;
      registrations->numStudents++;
      hashAddition(registrations->mainArray[registrations->numStudents-1],hashArray);
      if(hashArray->load >= HIGH_LOAD){
        reHashing(registrations,hashArray);
      }
      return SUCCESS ;
    }
    else{
      return FAIL;
    }

  }else if(registrations->numStudents == registrations->size){
    registrations->size += registrations->sizeIncrease ;
    temp = (StudentInfo **)realloc(registrations->mainArray,
                          registrations->size * sizeof(StudentInfo));

    if(temp != NULL){
      registrations->mainArray = temp;
      registrations->mainArray[registrations->numStudents] = 
                          (StudentInfo *)malloc(sizeof(StudentInfo));
      if(registrations->mainArray[registrations->numStudents] != NULL){
      registrations->mainArray[registrations->numStudents]->CoursesList = NULL ;

      *registrations->mainArray[registrations->numStudents] = input;
      registrations->numStudents++;

      /*Initialization of extended-arrays' vacant positions*/
      for(a=registrations->numStudents; a<registrations->size; a++){
        registrations->mainArray[a] = NULL ;
      }

      hashAddition(registrations->mainArray[registrations->numStudents-1],hashArray);
      if(hashArray->load >= HIGH_LOAD){
        reHashing(registrations,hashArray);
      }
      return SUCCESS ;
      }
      else{
        return FAIL ;
      }

    }else{
      return FAIL;
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

/* Student removal */
int courseListRemove(Course *list);
void hashRemove(StudentInfo *arrayInput,hashTable *hashArray);

int studentRemove(Array *registrations,hashTable *hashArray,
                  unsigned long int AEM,unsigned long int *comparisons){
  int r;
  StudentInfo **temp;

  r = Search(*registrations,AEM,comparisons) ;

  if(r != FAIL){
    hashRemove(registrations->mainArray[r],hashArray);
    courseListRemove(registrations->mainArray[r]->CoursesList);
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
    if(hashArray->load <= LOW_LOAD 
                    && (hashArray->hashSize / 2) >= hashArray->initialSize){
      reHashing(registrations,hashArray);
    }

    /* Size reduction */
    if(registrations->size - registrations->numStudents >= registrations->sizeIncrease){
      temp = (StudentInfo **)realloc(registrations->mainArray,
      (registrations->size - registrations->sizeIncrease) * sizeof(StudentInfo));

      if(temp == NULL && (registrations->size - registrations->sizeIncrease) != 0){
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

// Clear function 
void courseClear(Array *registrations,int pos);
int hashClear(hashTable *hashArray);

void clear(Array *registrations,hashTable *hashArray){
  int i ;

  hashClear(hashArray);
  if(registrations->mainArray != NULL){
    for(i=0; i<registrations->numStudents; i++){
      courseClear(registrations,i);
      free(registrations->mainArray[i]);
      registrations->mainArray[i] = NULL;
    }
    free(registrations->mainArray);
    registrations->mainArray = NULL;
    registrations->numStudents = 0 ;
    registrations->size = 0;
  }
}

void quit(Array *registrations,hashTable *hashArray){
  clear(registrations,hashArray);
  free(hashArray->bucketArray);
}

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

//PROJECT 2 *********************************************************
unsigned long int hash(char *str) {
  unsigned long int hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

unsigned long int hashFunction(char *str,int hashSize){
  return (hash(str) % hashSize) ;
}

Course *courseSearch(Array registrations,unsigned long int ID,int pos){
  Course *curr,*prev ;

  curr = registrations.mainArray[pos]->CoursesList ;
  prev = NULL ;
  for(; (curr != NULL); curr = curr->nextCourse){
    if(ID <= curr->ID){
      return prev;
    }
    prev = curr ;
  }
  return prev ;
}

int hashAddition(StudentInfo *arrayInput,hashTable *hashArray){
  unsigned long int pos ;
  StudentInfo *listNode ;

  pos = hashFunction(arrayInput->Fullname.lastName,hashArray->hashSize); 
  if(hashArray->bucketArray[pos].sentinel == NULL){
    hashArray->bucketArray[pos].sentinel = 
                    (StudentInfo *)malloc(sizeof(StudentInfo));
    if(hashArray->bucketArray[pos].sentinel != NULL){
      //Setting initial bucketArray pointers & values
      hashArray->bucketArray[pos].sentinel->next = arrayInput ;
      hashArray->bucketArray[pos].sentinel->prev = arrayInput ;
      hashArray->bucketArray[pos].sentinel->AEM = 0 ;
      //Setting first hash's pointers & values
      arrayInput->next = hashArray->bucketArray[pos].sentinel ;
      arrayInput->prev = hashArray->bucketArray[pos].sentinel ;
      hashArray->numHashes++ ;
      hashArray->bucketArray[pos].bucketSize++ ;
      hashArray->load =(double)(hashArray->numHashes / hashArray->hashSize) ;
      if(hashArray->bucketArray[pos].bucketSize > hashArray->largestBucket){
        hashArray->largestBucket = hashArray->bucketArray[pos].bucketSize ; 
      }
      return SUCCESS ;
    }
    else{
      return FAIL ;
    }
  }
  
  listNode = hashArray->bucketArray[pos].sentinel->next;
  for(; (listNode->AEM != 0); listNode=listNode->next){
    if(arrayInput->Fullname.lastName[0] < listNode->Fullname.lastName[0]){
      break; 
    }
    else if(strcmp(arrayInput->Fullname.lastName,listNode->Fullname.lastName) < 0){
      break ;
    }
    else if(strcmp(arrayInput->Fullname.lastName,listNode->Fullname.lastName) == 0
                                          && arrayInput->AEM < listNode->AEM){
      break;
    }
  }

  //New-node's next and prev nodes respectively
  arrayInput->next = listNode ;
  arrayInput->prev = listNode->prev ;
  /*New node is the new "next" of the 
  existing node and the "prev" of the current next node*/
  listNode->prev->next = arrayInput ;
  listNode->prev = arrayInput ;
  //Updating hash metrics 
  hashArray->numHashes++ ;
  hashArray->bucketArray[pos].bucketSize++ ;
  hashArray->load =(double)(hashArray->numHashes / hashArray->hashSize) ;
  if(hashArray->bucketArray[pos].bucketSize > hashArray->largestBucket){
    hashArray->largestBucket = hashArray->bucketArray[pos].bucketSize ; 
  }
  return SUCCESS ;
}

void hashRemove(StudentInfo *arrayInput,hashTable *hashArray){
  int pos ;
  StudentInfo *listNode ;

  pos = hashFunction(arrayInput->Fullname.lastName,hashArray->hashSize); 
  listNode = hashArray->bucketArray[pos].sentinel->next;
  for(; (listNode->AEM != 0); listNode=listNode->next){
    if(listNode->AEM == arrayInput->AEM){
      listNode->prev->next = listNode->next ;
      listNode->next->prev = listNode->prev ;
      hashArray->numHashes-- ;
      hashArray->bucketArray[pos].bucketSize --;
      hashArray->load =(double)(hashArray->numHashes / hashArray->hashSize) ;
    }
  }

  hashArray->largestBucket = 0 ;
  for(pos=0; pos<hashArray->hashSize; pos++){
    if(hashArray->bucketArray[pos].bucketSize > hashArray->largestBucket){
      hashArray->largestBucket = hashArray->bucketArray[pos].bucketSize ; 
    }
  }
}

int hashClear(hashTable *hashArray){
  int i ;
  Bucket *newArray;

  for(i=0; i<hashArray->hashSize; i++){
    free(hashArray->bucketArray[i].sentinel);
    hashArray->bucketArray[i].sentinel = NULL ;
    hashArray->bucketArray[i].bucketSize = 0;
  }
  newArray = (Bucket *)realloc(hashArray->bucketArray,
                      hashArray->initialSize * sizeof(Bucket));
  if(newArray != NULL){
    hashArray->bucketArray = newArray ;
    hashArray->hashSize = hashArray->initialSize ;
    hashArray->numHashes = 0;
    hashArray->load = 0;
    hashArray->largestBucket = 0;
    return SUCCESS ;
  }
  else{
    return F_MEM ;
  }
}

int reHashing(Array *registrations,hashTable *hashArray){
  Bucket *newArray = NULL;
  int i ;

  for(i=0; i<hashArray->hashSize; i++){
    free(hashArray->bucketArray[i].sentinel);
  }

  if(hashArray->load >= HIGH_LOAD){
    newArray = (Bucket *)realloc(hashArray->bucketArray,
                                hashArray->hashSize * 2 * sizeof(StudentInfo));
    hashArray->hashSize = 
            (newArray != NULL) ? hashArray->hashSize * 2 : hashArray->hashSize;
  }
  else if(hashArray->load <= LOW_LOAD 
                            && (hashArray->hashSize/2) >= hashArray->initialSize){
  
    newArray = (Bucket *)realloc(hashArray->bucketArray,
                              (hashArray->hashSize / 2) * sizeof(StudentInfo));
    hashArray->hashSize = 
            (newArray != NULL) ? hashArray->hashSize / 2 : hashArray->hashSize;
  }
  if(newArray != NULL){
    hashArray->bucketArray = newArray ;
    hashArray->numHashes = 0;
    hashArray->load = 0;
    hashArray->largestBucket = 0;
    for(i=0; i<hashArray->hashSize; i++){
      hashArray->bucketArray[i].sentinel = NULL ;
      hashArray->bucketArray[i].bucketSize = 0 ;
    }

    for(i=0; i<registrations->numStudents; i++){
      hashAddition(registrations->mainArray[i],hashArray);
    }
    hashArray->load = (hashArray->numHashes / hashArray->hashSize) ;
    return SUCCESS ;
  }
  else{
    return F_MEM ;
  }
}

int courseInsert(Array *registrations,StudentInfo input,
                unsigned long int ID,unsigned long int *comparisons){
  int pos ;
  Course *course,*newCourse ;

  pos = Search(*registrations,input.AEM,comparisons) ;

  if(pos != FAIL){
    if(registrations->mainArray[pos]->CoursesList == NULL){
      registrations->mainArray[pos]->CoursesList = 
                                        (Course*)malloc(sizeof(Course));
      if(registrations->mainArray[pos]->CoursesList != NULL){
        registrations->mainArray[pos]->CoursesList->ID = ID ;
        registrations->mainArray[pos]->CoursesList->nextCourse = NULL ;
        return SUCCESS ;
      }else{
        return F_MEM ;
      }
    }
    else{
      course = courseSearch(*registrations,ID,pos);
      if(course == NULL){
        if(registrations->mainArray[pos]->CoursesList->ID > ID){
          newCourse = (Course *)malloc(sizeof(Course));
          if(newCourse != NULL){
            newCourse->ID = ID;
            newCourse->nextCourse =
                registrations->mainArray[pos]->CoursesList;
            registrations->mainArray[pos]->CoursesList = newCourse ;
            return SUCCESS ;
          }else{
            return F_MEM ;
          }
        }
        else{
          return F_ID ;
        }
      }
      else if(course->nextCourse == NULL || course->nextCourse->ID > ID){
        newCourse = (Course *)malloc(sizeof(Course));
        if(newCourse != NULL){
          newCourse->nextCourse = course->nextCourse ;
          course->nextCourse = newCourse ;
          newCourse->ID = ID;
          return SUCCESS ;
        }
        else{
          return F_MEM ;
        }
      }
      else if(course->nextCourse->ID == ID){
        return F_ID;
      }
    }
  }
  else{
    return F_AEM ;
  }
  return -2;
}

int courseRemoval(Array *registrations,StudentInfo input,
                  unsigned long int ID,unsigned long int *comparisons){
  Course *prev, *curr ;
  int pos ;

  pos = Search(*registrations,input.AEM,comparisons);

  if(pos != FAIL){
    prev = courseSearch(*registrations,ID,pos);
    if(prev == NULL){
      if(registrations->mainArray[pos]->CoursesList != NULL){
        curr = registrations->mainArray[pos]->CoursesList->nextCourse ;
        if(registrations->mainArray[pos]->CoursesList->ID == ID){
          free(registrations->mainArray[pos]->CoursesList);
          registrations->mainArray[pos]->CoursesList = curr ;
          return SUCCESS ;
        }
      }  
    }
    else if(prev->nextCourse != NULL && ID == prev->nextCourse->ID){
      curr = prev->nextCourse->nextCourse ;
      free(prev->nextCourse);
      prev->nextCourse = curr;
      return SUCCESS ;
    }
    return F_ID ;
  }
  else{
    return F_AEM;
  }
}


int courseListRemove(Course *list){
  if(list != NULL && list->nextCourse == NULL){
    free(list);
    return 0;
  }
  else if(list != NULL){
    courseListRemove(list->nextCourse) ;
    free(list) ;
    return 0;
  }
  return -2 ;
}

void courseClear(Array *registrations,int pos){
  Course *curr,*prev ;

  curr = registrations->mainArray[pos]->CoursesList ;
  while(curr != NULL){
    prev = curr ;
    curr=curr->nextCourse ;
    free(prev);
  }
  registrations->mainArray[pos]->CoursesList = NULL ;
}

// Course search for a student
int isReg(Array *registrations,StudentInfo input,unsigned long int ID,
                                        unsigned long int *comparisons){
  int pos ;
  Course *course ;

  pos = Search(*registrations,input.AEM,comparisons);
  if(pos != FAIL){
    course = courseSearch(*registrations,ID,pos);
    if(course == NULL || course->nextCourse == NULL){
      if(registrations->mainArray[pos]->CoursesList != NULL 
              && registrations->mainArray[pos]->CoursesList->ID == ID){
        return SUCCESS ;
      }
    }
    else if(course->nextCourse->ID == ID){
      return SUCCESS ;
    }
    return F_ID ;
  }
  else{
    return F_AEM ;
  }
  return -2 ;
}

//Course list print
void list_courses(Array registrations,StudentInfo input,
                         unsigned long int *comparisons){
  int pos ;
  Course *curr ;

  pos = Search(registrations,input.AEM,comparisons);
  if(pos != FAIL){
    printf("\nL-OK %s ",registrations.mainArray[pos]->Fullname.lastName);
    printf("%lu\n",input.AEM);

    curr = registrations.mainArray[pos]->CoursesList ;
    for(; (curr != NULL); curr = curr->nextCourse){
      printf("%lu\n",curr->ID);
    }
  }
  else{
    printf("\nL-NOK %lu\n",input.AEM);
  }
}

int findByName(hashTable hashArray,StudentInfo input,
                        unsigned long int *comparisons){
  int pos ;
  StudentInfo *curr ;

  capital(input.Fullname.lastName);
  pos = hashFunction(input.Fullname.lastName,hashArray.hashSize);
  
  if(hashArray.bucketArray[pos].sentinel != NULL){
    curr = hashArray.bucketArray[pos].sentinel->next ;
    for(; (curr->AEM != 0); curr=curr->next){
      (*comparisons)++ ;
      if(strcmp(curr->Fullname.lastName,input.Fullname.lastName) == 0){
        printf("\nN-OK %s\n", curr->Fullname.lastName);
        for(; (curr->AEM != 0); curr=curr->next){
          (*comparisons)++ ;
          if(strcmp(curr->Fullname.lastName,input.Fullname.lastName) == 0){
            printf("%s %lu %u\n",curr->Fullname.firstName,curr->AEM,curr->FailedCS);
          }
          else{
            break;
          }
        }
        return SUCCESS ;
      }
    }
    printf("\nN-NOK %s\n",input.Fullname.lastName);
    return FAIL ;
  }
  printf("\nN-NOK %s\n",input.Fullname.lastName);
  return FAIL ;
}

void printByName(hashTable hashArray,StudentInfo input){
  int pos;
  StudentInfo *curr ;

  printf("\n##\n");
  printf("%d %.0lf",hashArray.hashSize,hashArray.numHashes);
  printf(" %.2lf %d\n",hashArray.load,hashArray.largestBucket);

  for(pos=0; pos<hashArray.hashSize; pos++){
    if(hashArray.bucketArray[pos].sentinel != NULL
                    && hashArray.bucketArray[pos].sentinel->next->AEM != 0){

      printf("%d %d ",pos,hashArray.bucketArray[pos].bucketSize);
      curr=hashArray.bucketArray[pos].sentinel->next ;
      for(; (curr->AEM != 0); curr = curr->next){
        if(curr->next->AEM == 0){
          printf("[ %lu %s ",curr->AEM,curr->Fullname.firstName);
          printf("%s %u ]",curr->Fullname.lastName,curr->FailedCS);
        }else{
          printf("[ %lu %s ",curr->AEM,curr->Fullname.firstName);
          printf("%s %u ] ",curr->Fullname.lastName,curr->FailedCS);
        }
      }
    }
    else{
      printf("%d %d",pos,hashArray.bucketArray[pos].bucketSize);
    }
    printf("\n\n");
  }
}

int main(int argc, char *argv[]){
  Array registrations ;               
  StudentInfo input = {0,{{'\0'},{'\0'}},0,0} ;
  hashTable hashArray = {0} ;
  int k ;
  unsigned long int comparisons = 0;
  unsigned long int ID ;
  char command ;
  char *tempStr1,*tempStr2 ;

  if(argc != 4){
    return 42;
  }

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

  //Setting up hashtable main info
  hashArray.hashSize = atoi(argv[3]);
  hashArray.load = 0 ;
  hashArray.numHashes = 0;
  hashArray.initialSize = atoi(argv[3]) ;
  hashArray.bucketArray = (Bucket *)malloc(atoi(argv[3]) * sizeof(Bucket));
  for(k=0; k<hashArray.hashSize; k++){
    hashArray.bucketArray[k].sentinel = NULL ;
    hashArray.bucketArray[k].bucketSize = 0;
  }


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
        }
        else{
          if(studentAddition(&registrations,input,&hashArray,&comparisons) 
                                                                   == SUCCESS){
            printf("\nA-OK %lu,",input.AEM);
            printf(" %d %d\n",registrations.numStudents,registrations.size);
          }
          else{
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
          }
          else{
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
        if(input.AEM != 0){
          if(studentRemove(&registrations,&hashArray,input.AEM,&comparisons) 
                                                                  == SUCCESS){
            printf("\nR-OK %lu, %d",input.AEM,registrations.numStudents);
            printf(" %d\n",registrations.size);
          }
          else{
            printf("\nR-NOK %lu, %d",input.AEM,registrations.numStudents);
            printf(" %d\n",registrations.size);
          }
        }
        else{
          printf("\nInvalid input.\n");
        }
        break;
      }
      case 'f':{
        scanf(" %lu",&input.AEM);
        if(input.AEM != 0){
          k = Search(registrations,input.AEM,&comparisons);
          if(k != FAIL){
            printf("\nF-OK %s",registrations.mainArray[k]->Fullname.firstName);
            printf(" %s",registrations.mainArray[k]->Fullname.lastName);
            printf(" %d\n",registrations.mainArray[k]->FailedCS);
          }
          else{
            printf("\nF-NOK %lu\n",input.AEM);
          }
          fprintf(stderr,"\n$%lu\n",comparisons);
        }
        break;
      }
      case 'p':{
        print(&registrations);
        break;
      }
      case 'c':{
        clear(&registrations,&hashArray);
        printf("\nC-OK\n");
        break;
      }
      case 'q':{
        quit(&registrations,&hashArray);
        return 0;
      }
      case 'g':{
        scanf(" %lu %lu",&input.AEM, &ID);
        if(input.AEM != 0 && ID != 0){
          k = courseInsert(&registrations,input,ID,&comparisons);
          if(k == SUCCESS){
            printf("\nG-OK %lu %lu\n",input.AEM, ID);
          }
          else if(k == F_AEM){
            printf("\nG-NOK %lu\n",input.AEM);
          }
          else if(k == F_ID){
            printf("\nG-NOK %lu\n",ID);
          }
          else if(k == F_MEM){
            printf("\nG-NOK MEM\n");
          }
        }
        else{
          printf("\nInvalid input.\n");
        }
        break ;
      }
      case 'u':{
        scanf(" %lu %lu",&input.AEM, &ID);
        if(input.AEM != 0 && ID != 0){
          k = courseRemoval(&registrations,input,ID,&comparisons);
          if(k == SUCCESS){
            printf("\nU-OK %lu %lu\n",input.AEM,ID);
          }
          else if(k == F_AEM){
            printf("\nU-NOK %lu\n",input.AEM);
          }
          else if(k == F_ID){
            printf("\nU-NOK %lu\n",ID);
          }
        }
        else{
          printf("\nInvalid input.\n");
        }
        break;
      }
      case 'i':{
        scanf(" %lu %lu",&input.AEM, &ID);
        if(input.AEM != 0 && ID != 0){
          k = isReg(&registrations,input,ID,&comparisons);
          if(k == SUCCESS){
            printf("\nYES\n");
          }
          else if(k == F_ID){
            printf("\nNO\n");
          }
          else if(k == F_AEM){
            printf("\nI-NOK %lu\n",input.AEM);
          }
        }
        else{
          printf("\nInvalid input.\n");
        }
        break ;
      }
      case 'l':{
        scanf(" %lu",&input.AEM);
        list_courses(registrations,input,&comparisons);
        break ;
      }
      case 'n':{
        scanf(" %ms",&tempStr1);
        if(strlen(tempStr1) >= NAMESIZE){
          printf("\nTrimmed name(s).\n");
          strncpy(input.Fullname.lastName,tempStr1,NAMESIZE-1);
        }else{
          strcpy(input.Fullname.lastName,tempStr1);
        }
        free(tempStr1);
        tempStr1 = NULL;
        findByName(hashArray,input,&comparisons);
        fprintf(stderr,"\n@%lu\n",comparisons);
        break;
      }
      case 't':{
        printByName(hashArray,input);
        break;
      }
      default:{
        printf("\nInvalid command\n");
      } 
    }
    comparisons = 0;
  }
}
