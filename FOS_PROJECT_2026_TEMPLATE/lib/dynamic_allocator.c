/*
 * dynamic_allocator.c
 *
 * Created on: Sep 21, 2023
 *    Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [0] GIVEN FUNCTIONS
//==================================
// [1] INITIALIZE BUDDY FREE LISTS:
//==================================
void buddy_init()
{
         //panic("buddy_init is not implemented yet");

         for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
         {
                   LIST_INIT(&(freeBlockLists[l]));
         }
}
//====================================
// [2] PRINT SIZE OF FREE LISTS:
//====================================
void PrintFreeBlockLists()
{
         for (int i = BUDDY_LOWER_LEVEL; i <= BUDDY_UPPER_LEVEL; ++i)
         {
                   cprintf("Level#%d: %d\n", i, LIST_SIZE(&freeBlockLists[i]));
         }
}
/************************************************************************************/
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] GET BLOCK SIZE:
//===========================
__inline__ uint32 get_block_size(void *va)
{
        //TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [1] get_block_size
        //Your code is here
        //Comment the following line
        //panic("get_block_size() not implemented");
        struct BlockMetaData *header = (struct BlockMetaData *)((uint32)va - META_DATA_SIZE);
                return BLOCK_SIZE(header->level);

}
//===========================
// [2] ALLOCATE BLOCK:
//===========================
//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [2] alloc_block
void *alloc_block(uint32 requiredSize)
{
          if(requiredSize == 0)
                  return NULL;
    uint32 total_size = requiredSize + sizeof(struct BlockMetaData);
    struct BlockMetaData *search = NULL ;
    LIST_FOREACH(search,&freeBlockLists,BlockMetaData)
    {
       uint32 block_size = get_block_size((void *)search);
       if(block_size >= total_size)
       {
        if(block_size > total_size+ sizeof(struct BlockMetaData))
        {
                  struct BlockMetaData *newblock =(struct BlockMetaData)((char*)search + total_size);
                  newblock->is_free=1;
                  newblock-> level = block_size-total_size;
                  LIST_INSERT_AFTER(&freeBlockLists,search,newblock);
        }
        search->level = total_size;
        search->is_free = 0;
        LIST_REMOVE(&freeBlockLists,search);
        return (void*)(char*)search+sizeof(struct BlockMetaData);
       }
       return NULL;
    }
       panic("alloc_block() not implemented");
}




//===================================================
// [3] FREE BLOCK WITH COALESCING:
//===================================================

void free_block(void *va)
{
        //TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [3] free_block
        //Your code is here
        //Comment the following line
        //panic("free_block() not implemented");
        if (va == NULL)
                       return;

          struct BlockMetaData *current = (struct BlockMetaData *)((uint32)va - META_DATA_SIZE);
          if (current->is_free)
                  return;

              int curLevel = current->level;


              while (curLevel < BUDDY_UPPER_LEVEL)
              {
                     struct BlockMetaData *partner =
                             (struct BlockMetaData *)((uint32)current ^ BLOCK_SIZE(curLevel));

                      if (!(partner->is_free && partner->level == curLevel))
                               break;

                      LIST_REMOVE(&freeBlockLists[curLevel], partner);

                      if ((uint32)partner < (uint32)current)
                               current = partner;
                      curLevel++;
              }

              current->level = curLevel;
              current->is_free = 1;
              LIST_INSERT_HEAD(&freeBlockLists[curLevel], current);

}
void *alloc_block(uint32 requiredSize)
{cprintf("STAFF alloc_block called\n");
    if (requiredSize == 0)
        return NULL;

    uint32 total_size = requiredSize + META_DATA_SIZE;

    int level = BUDDY_LOWER_LEVEL;
    while (level <= BUDDY_UPPER_LEVEL && BLOCK_SIZE(level) < total_size)
        level++;
    if (level > BUDDY_UPPER_LEVEL)
        return NULL;

    int cur = level;
    while (cur <= BUDDY_UPPER_LEVEL && LIST_EMPTY(&freeBlockLists[cur]))
        cur++;
    if (cur > BUDDY_UPPER_LEVEL)
        return NULL;

    struct BlockMetaData *blk = LIST_FIRST(&freeBlockLists[cur]);
    LIST_REMOVE(&freeBlockLists[cur], blk);


    while (cur > level)
    {
      cur--;
      struct BlockMetaData *buddy =
         (struct BlockMetaData *)((uint32)blk + BLOCK_SIZE(cur));
      buddy->level = cur;
      buddy->is_free = 1;
      LIST_INSERT_HEAD(&freeBlockLists[cur], buddy);
    }

    blk->level = level;
    blk->is_free = 0;
    return (void *)((uint32)blk + META_DATA_SIZE);
}

//=================================================================================//
//============================== BONUS FUNCTION===================================//
//=================================================================================//

//===========================
// [1] REALLOCATE BLOCK:
//===========================
void *realloc_block(void* va, uint32 newSize)
{
   //TODO: [PROJECT'26.BONUS#1] KERNEL REALLOC - realloc_block
   //Your code is here
   //Comment the following line
   panic("realloc_block() Not implemented yet");
}

