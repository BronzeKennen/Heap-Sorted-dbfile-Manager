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

#define RECORDS_PER_BLOCK (BF_BLOCK_SIZE - sizeof(HP_block_info)) / sizeof(Record)

int HP_CreateFile(char *fileName){
    int file_desc;
    BF_Block *block;
    
    BF_Block_Init(&block);
    CALL_BF(BF_OpenFile(fileName, &file_desc));
    CALL_BF(BF_AllocateBlock(file_desc, block));

    void *test = BF_Block_GetData(block); //GET POINTER TO BEGINNING OF BLOCK

    HP_info *data = test; //MOVE TO AN HP_INFO POINTER  
    data->type = Heap; // SET DATA FOR HP_INFO
    data->last_block_id = 0;
    
    BF_Block_SetDirty(block); //SET DIRTY AND UNPIN TO WRITE TO DISK
    
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc));
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){

    BF_Block *block;
    BF_Block_Init(&block);
    BF_OpenFile(fileName,file_desc);
    BF_GetBlock(*file_desc,0,block);

    void* data = BF_Block_GetData(block);

    HP_info *hpInfo = data;
    BF_Block_Destroy(&block);
    
    return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    int block_num;
    BF_GetBlockCounter(file_desc,&block_num);
    BF_Block *block;
    BF_Block_Init(&block);

    for(int i = 0; i < block_num; i++) {

        BF_GetBlock(file_desc,i,block);
        // printf("UNPINNING BLOCK %d\n",i);
        BF_Block_SetDirty(block);
        BF_ErrorCode code = BF_UnpinBlock(block);
    
    }

    BF_CloseFile(file_desc);
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    BF_Block *block;
    BF_Block_Init(&block);

    BF_GetBlock(file_desc,hp_info->last_block_id,block);
    char *data = BF_Block_GetData(block);
   
    HP_block_info* blockInfo;
   
    if(hp_info->last_block_id != 0) {

        data = data + BF_BLOCK_SIZE - sizeof(HP_block_info);
        void *data2 = data;
        blockInfo = data2;
    
    }
    if(hp_info->last_block_id == 0 || blockInfo->num_of_records == RECORDS_PER_BLOCK) {
        printf("CREATING BLOCK WITH ID %d\n",hp_info->last_block_id);
        BF_Block *newBlock;
        BF_Block_Init(&newBlock);
        BF_AllocateBlock(file_desc,newBlock);
        HP_block_info blockInfoNew = {0,NULL}; //set block info
        
        char* blockData = BF_Block_GetData(newBlock);
        blockData = blockData + (BF_BLOCK_SIZE - sizeof(HP_block_info));
        
        memcpy(blockData,&blockInfoNew,sizeof(HP_block_info)); //go to end of block and copy the info created
        if(hp_info->last_block_id != 0) {
            blockInfo->next_block = newBlock;
            memcpy(data,blockInfo,sizeof(HP_block_info));
            BF_UnpinBlock(block);
        }
        hp_info->last_block_id++;
        block = newBlock;

    }
    char *infoData = BF_Block_GetData(block);

    infoData = infoData + BF_BLOCK_SIZE - sizeof(HP_block_info);
    void *infoData2 = infoData;
    
    blockInfo = infoData2;
    void* recData = BF_Block_GetData(block);
    Record *rec = recData;
    rec[blockInfo->num_of_records] = record;

    printf("Record num %d\n",blockInfo->num_of_records);
    blockInfo->num_of_records++;
    BF_Block_SetDirty(block);
    return 0;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
    
    return -1;
}