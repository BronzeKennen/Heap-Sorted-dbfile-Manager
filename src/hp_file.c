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
    
    BF_Block_Init(&block); //Create initial block with hp_info 
    CALL_BF(BF_OpenFile(fileName, &file_desc));
    CALL_BF(BF_AllocateBlock(file_desc, block));

    HP_info *data = (HP_info*)BF_Block_GetData(block); //Get pointer to beginning of block

    data->type = Heap; // Set data for hp_info
    data->last_block_id = 0;
    
    BF_Block_SetDirty(block); //Set dirty and unpin
    
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

    for(int i = 0; i < block_num; i++) { //Unpin all blocks (file cannot close otherwise)

        BF_GetBlock(file_desc,i,block);
        BF_ErrorCode code = BF_UnpinBlock(block);
    
    }

    BF_Block_Destroy(&block);   

    BF_CloseFile(file_desc);
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    BF_Block *block;
    BF_Block_Init(&block);

    BF_GetBlock(file_desc,hp_info->last_block_id,block); //Getr last block
    char *data = BF_Block_GetData(block);
   
    HP_block_info* blockInfo;
   
    if(hp_info->last_block_id != 0) {
        blockInfo = getBlockInfo(block); //first block does not have hp_block_info
    }
    if(hp_info->last_block_id == 0 || blockInfo->num_of_records == RECORDS_PER_BLOCK) { //if all blocks are full, create a new one
        BF_Block *newBlock;
        BF_Block_Init(&newBlock);
        BF_AllocateBlock(file_desc,newBlock); 
        HP_block_info blockInfoNew = {0,NULL}; //Init new block info
        
        char* blockData = BF_Block_GetData(newBlock);
        blockData = blockData + (BF_BLOCK_SIZE - sizeof(HP_block_info));
        memcpy(blockData,&blockInfoNew,sizeof(HP_block_info)); //go to end of block and copy the info created
        
        BF_Block_SetDirty(block);
        if(hp_info->last_block_id != 0) {
            blockInfo->next_block = newBlock; //each block's block_info has a pointer to the next one
            BF_Block_SetDirty(block); //we are done with old block so unpin
            BF_UnpinBlock(block);
        }
        hp_info->last_block_id++; //More blocks!!!
        BF_UnpinBlock(newBlock);
        BF_Block_Destroy(&newBlock); //we are done with new block (for now)
        
    }

    BF_GetBlock(file_desc,hp_info->last_block_id,block); //get the last block created
    blockInfo = getBlockInfo(block);
    void* recData = BF_Block_GetData(block);
    Record *rec = recData;
    rec[blockInfo->num_of_records] = record; //place the new record after the last one inserted

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
    
    for(int i = 1; i <= hp_info->last_block_id; i++) {
        BF_ErrorCode code = BF_GetBlock(file_desc,i,block);
        if(code != BF_OK)
            return -1;

        blockInfo = getBlockInfo(block);

        void* data = BF_Block_GetData(block);
        Record *recs = data;

        for(int j = 0; j < blockInfo->num_of_records; j++) { 
            if(recs[j].id == value) { 
                printRecord(recs[j]);
            }
        }
        BF_UnpinBlock(block);
    }
    BF_Block_Destroy(&block);
    
    return hp_info->last_block_id;
}

HP_block_info* getBlockInfo(BF_Block* block) {
    char* blockData = BF_Block_GetData(block); //cast to char pointer to increment by 1
    blockData = blockData + BF_BLOCK_SIZE - sizeof(HP_block_info); //get where we want
    void* blockData2 = blockData; //cast it to void pointer
    HP_block_info *blockInfo = blockData2; // and then cast it to HP_block_info (cant directly cast it from char*)
    return blockInfo;
}