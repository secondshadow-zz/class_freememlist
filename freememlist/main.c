//
//  main.c
//  freememlist
//
//  Created by Kevin Carter on 5/14/14.
//  Copyright (c) 2014 Kevin Carter. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "llist.h"

typedef struct pageDef {
    long start;
    long end;
}pageDef;

typedef enum e_commandid {
    RESERVED,
    INIT,
    ALLOCATE,
    FREE,
    PRINT
} commandId;

typedef struct commandStruct {
    char command[50];
    commandId id;
    int requiresNumericSecondArg;
} commandStruct;

commandStruct commands[] = {
    {"init",INIT,1},
    {"allocate",ALLOCATE,1},
    {"free",FREE,1},
    {"print",PRINT,0},
    {0,0,0}
};


void *pageCleanupFunc(void *page) {
//    printf("Freeing: %li\n", ((pageDef *) page)->);
    free(page);
    return NULL;
}

void initializeFml(LinkedList **freeList, LinkedList **usedList,int blockCount) {
    int i;
    pageDef *page;
    if(*freeList!=NULL) {
        ll_destroy(*freeList, pageCleanupFunc);
    }
    
    if(*usedList!=NULL) {
        ll_destroy(*usedList, pageCleanupFunc);
    }
    
    *freeList = ll_create();
    *usedList = ll_create();
    
    page = malloc(sizeof(*page));
    page->start=0;
    page->end=blockCount-1;
    ll_append(*freeList, page);
}

int prompt(char *buffer) {
    printf("$ ");
    return scanf("%s",buffer);
}

void *printBlock(void *data,void *param) {
    pageDef *page = (pageDef *) data;
    printf("%li-%li (size %li)\n",page->start,page->end,(page->end-page->start)+1);
    return data;
}

void printData(LinkedList *freeList, LinkedList *usedList) {
    printf("Free memory:\n\n");
    ll_mapInline(freeList, NULL, printBlock);
    
    printf("\nUsed memory:\n\n");
    ll_mapInline(usedList, NULL, printBlock);
}

int isFreeBlockBigEnough(void *data, void *param) {
    long requestedSize = *(long *) param;
    pageDef *page = (pageDef *) data;
    
    return (page->end-page->start+1) >= requestedSize;
}

int performAllocation(LinkedList *freeList, LinkedList *usedList, long requestedSize, int *acquiredAddress) {
    int retval = 1;
    pageDef *acquiredpage;
    pageDef *newPage;
    LinkedListEntry *entry = ll_search(freeList, &requestedSize, isFreeBlockBigEnough);
    
    if(entry!=NULL) {
        acquiredpage=(pageDef *)entry->data;
        newPage=malloc(sizeof(*newPage));
        newPage->start=acquiredpage->start;
        newPage->end=newPage->start+requestedSize-1;
        ll_append(usedList, newPage);
        
        acquiredpage->start+=requestedSize;
        if(acquiredpage->start > acquiredpage->end) {
            ll_remove(entry, pageCleanupFunc);
        }
    } else {
        retval = 0;
    }
    return retval;
}

void executeCommand(LinkedList **freeList,
                    LinkedList **usedList,
                    commandStruct *command,
                    int numericArg){
    int acquiredAddress=0;
    switch(command->id) {
        case INIT:
            initializeFml(freeList, usedList, numericArg);
            printf("Initialization complete\n\n");
            break;
        case ALLOCATE:
            if(performAllocation(*freeList,*usedList,numericArg,&acquiredAddress)){
                printf("your address is %i\n\n",acquiredAddress);
            } else {
                printf("error, no contiguous available\n\n");
            }
            break;
//        case FREE:
//            if(performFree(*freeList,usedList,numericArg)) {
//                printf("ok\n\n");
//            } else {
//                printf("error, not an allocated block\n\n");
//            }
//            break;
        case PRINT:
            printData(*freeList,*usedList);
            break;
        case RESERVED:
        default:
            printf("Invalid command. Try again.\n\n");
    }
}

int main(int argc, const char * argv[])
{
    commandStruct *currentCommand=NULL;
    char command[1024];
    LinkedList *freeList=NULL;
    LinkedList *usedList=NULL;
    int blockCount;
    int i;
    int numericArg=0;
    
    while((EOF!=prompt(command))) {
        for(i=0;
            commands[i].id!=0 && strcmp(command,(currentCommand=&commands[i])->command)!=0;
            i++) {
            ;
            printf("DEBUG: %s\n", currentCommand->command);
        }
        if(currentCommand!=NULL){
            if(currentCommand->requiresNumericSecondArg) {
                scanf("%i",&numericArg);
            }
            
            executeCommand(&freeList,&usedList,currentCommand,numericArg);
        }
    }
    
    initializeFml(&freeList,&usedList, blockCount);
    
    
    
    return 0;
}

