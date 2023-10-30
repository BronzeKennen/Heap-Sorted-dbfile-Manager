#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
    int filedesc;
    HP_info info = { Heap, 10, 2};
    BF_Block *block,*block2;
    int blocknum;


    CALL_BF(BF_CreateFile(fileName));

    CALL_BF(BF_OpenFile(fileName, &filedesc));

    BF_Block_Init(&block);

    memcpy(block, &info, sizeof(info)); //COPIES FROM HP_INFO STRUCT TO THE RECENTLY INITIALIZED BLOCK
    CALL_BF(BF_AllocateBlock(filedesc, block)); //ALLOCATES THE BLOCK WITH HP_INFO DATA TO THE HEAP FILE
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block)); //NOTE: BEFORE CLOSING FILE ALL BLOCKS MUST BE UNPINNED OTHERWISE YOU GET ERROR
    CALL_BF(BF_CloseFile(filedesc));
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
    HP_info* hpInfo;    
    BF_Block *block;
    int blocknum;
    BF_OpenFile(fileName, file_desc);
    BF_GetBlockCounter(*file_desc, &blocknum); //THIS RETURNS 1 
    BF_ErrorCode error = BF_GetBlock(*file_desc,0,block); //NO WORKEY <-
    BF_PrintError(error);
    return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    return -1;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
    return -1;
}

