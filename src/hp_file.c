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
    int fd1;
    BF_Block *block;
    
    BF_Block_Init(&block);
    CALL_BF(BF_OpenFile(fileName, &fd1));
    CALL_BF(BF_AllocateBlock(fd1, block));

    void *test = BF_Block_GetData(block); //GET POINTER TO BEGINNING OF BLOCK

    HP_info *data = test; //MOVE TO AN HP_INFO POINTER  
    data->type = Heap; // SET DATA FOR HP_INFO
    data->last_block_id = 0;
    data->records_per_block = 20;
    BF_Block_SetDirty(block); //SET DIRTY AND UNPIN TO WRITE TO DISK
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fd1));
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
    BF_Block *block;
    BF_Block_Init(&block);
    BF_OpenFile(fileName,file_desc);
    BF_GetBlock(*file_desc,0,block);
    void* test = BF_Block_GetData(block);
    HP_info *hpInfo = test;
    BF_Block_Destroy(&block);
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

