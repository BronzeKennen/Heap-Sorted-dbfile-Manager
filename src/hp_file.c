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

    HP_info *data = (HP_info*)BF_Block_GetData(block); //GET POINTER TO BEGINNING OF BLOCK

    // HP_info *data = test; //MOVE TO AN HP_INFO POINTER  
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

    BF_Block_Destroy(&block);   

    BF_CloseFile(file_desc);
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    BF_Block *block;
    BF_Block_Init(&block);

    BF_GetBlock(file_desc,hp_info->last_block_id,block);
    char *data = BF_Block_GetData(block);
   
    HP_block_info* blockInfo;
   
    if(hp_info->last_block_id != 0) {
        blockInfo = getBlockInfo(block);
    }
    if(hp_info->last_block_id == 0 || blockInfo->num_of_records == RECORDS_PER_BLOCK) {
        // printf("CREATING BLOCK WITH ID %d\n",hp_info->last_block_id);
        BF_Block *newBlock;
        BF_Block_Init(&newBlock);
        BF_AllocateBlock(file_desc,newBlock);
        HP_block_info blockInfoNew = {0,NULL}; //set block info
        
        char* blockData = BF_Block_GetData(newBlock);
        blockData = blockData + (BF_BLOCK_SIZE - sizeof(HP_block_info));
        
        memcpy(blockData,&blockInfoNew,sizeof(HP_block_info)); //go to end of block and copy the info created
        BF_Block_SetDirty(newBlock);
        if(hp_info->last_block_id != 0) {
            blockInfo->next_block = newBlock;
            memcpy(data,blockInfo,sizeof(HP_block_info));
            BF_UnpinBlock(block);
            BF_Block_SetDirty(block); //////////////////????????????????????????????????
        }
        hp_info->last_block_id++;
        block = newBlock;
        // printf("NewBlock = %p --- last_block_id = %p\n", newBlock, hp_info->last_block_id);
        // newBlock = NULL;
        // BF_UnpinBlock(newBlock);
        // BF_Block_Destroy(&newBlock);
    }
    char *infoData = BF_Block_GetData(block);

    infoData = infoData + BF_BLOCK_SIZE - sizeof(HP_block_info);
    void *infoData2 = infoData;
    
    blockInfo = infoData2;
    void* recData = BF_Block_GetData(block);
    Record *rec = recData;
    rec[blockInfo->num_of_records] = record;

    // printf("Record num %d\n",blockInfo->num_of_records); //to print which record is being written
    blockInfo->num_of_records++;
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);

    BF_Block_Destroy(&block);
    return 0;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
   
    BF_Block *block;
    BF_Block_Init(&block);

    char *data = BF_Block_GetData(block);
        
    HP_block_info* blockInfo;
    
    for(int i = 1; i <= hp_info->last_block_id; i++) { //SEARCH EACH BLOCK
        BF_ErrorCode code = BF_GetBlock(file_desc,i,block);

        // IF ERROR OCCURS
        if(code != BF_OK)
            return -1;

        blockInfo = getBlockInfo(block);

        void* data = BF_Block_GetData(block);
        Record *recs = data;

        for(int j = 0; j < blockInfo->num_of_records; j++) { //SEARCH EACH ENTRY
            if(recs[j].id == value) { //IF MATCHING ID PRINT IT
                printRecord(recs[j]);
            }
        }
        BF_UnpinBlock(block);
    }
    BF_Block_Destroy(&block);
    
    // return number of blocks read while searching 
    return hp_info->last_block_id;
}

HP_block_info* getBlockInfo(BF_Block* block) {
    char* blockData = BF_Block_GetData(block); //cast to char pointer to increment by 1
    blockData = blockData + BF_BLOCK_SIZE - sizeof(HP_block_info); //get where we want
    void* blockData2 = blockData; //cast it to void pointer
    HP_block_info *blockInfo = blockData2; // and then cast it to HP_block_info (cant directly cast it from char*)
    return blockInfo;
}