/*
 * test_lists_managment.c

 *
 *  Created on: Oct 6, 2022
 *  Updated on: Sept 20, 2023
 *      Author: HP
 */
#include <inc/queue.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/dynamic_allocator.h>
#include <inc/memlayout.h>
#include <kern/mem/memory_manager.h>

//NOTE: ALL tests in this file shall work with USE_KHEAP = 0

/***********************************************************************************************************************/
#define Mega  (1024*1024)
#define kilo (1024)
#define numOfLevels (BUDDY_UPPER_LEVEL - BUDDY_LOWER_LEVEL + 1)

short* startVAsInit[DYN_ALLOC_MAX_BLOCK_SIZE + 1] ;
short* endVAsInit[DYN_ALLOC_MAX_BLOCK_SIZE + 1] ;
int initBlkCnt = 0;

__inline__ uint8 IDX(uint32 size)
{
	size>>= BUDDY_LOWER_LEVEL;
	int index = 0;
	while ((size>>=1) != 0)
	{
		index++;
	}
	return index;
}

int check_blocks_in_page(void* pageVA, struct BlockMetaData* expectedBlocks, uint32 numOfBlocks)
{
	int isCorrect = 1;
	struct BlockMetaData* curBlk = (struct BlockMetaData*) pageVA;
	uint32 endVA = (uint32)pageVA + PAGE_SIZE;
	uint32 idx = 0;
	while ((uint32)curBlk < endVA)
	{
		if (curBlk->is_free != expectedBlocks[idx].is_free || curBlk->level != expectedBlocks[idx].level)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "check_blocks_in_page#1: WRONG! invalid content of block#%d.\nExpected (level, status) = (%d, %d), Actual (level, status) = (%d, %d)\n",
					idx, expectedBlocks[idx].level, expectedBlocks[idx].is_free, curBlk->level, curBlk->is_free);
			isCorrect = 0;
			break;
		}
		idx++;
		curBlk = (struct BlockMetaData*)((uint32)curBlk + BLOCK_SIZE(curBlk->level));
	}
	if (isCorrect && idx != numOfBlocks)
	{
		cprintf_colored(TEXT_TESTERR_CLR, "check_blocks_in_page#2: WRONG! invalid num of blocks. Expected = %d, Actual = %d\n", numOfBlocks, idx);
		isCorrect = 0;
	}
	return isCorrect;
}

int check_free_block_lists(uint32 expectedSizes[BUDDY_UPPER_LEVEL+1], uint32* pagesVAs, uint32 numOfPages)
{
	int isCorrect = 1;
	for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
	{
		//Check number of blocks
		if (LIST_SIZE(&freeBlockLists[l]) != expectedSizes[l])
		{
			cprintf_colored(TEXT_TESTERR_CLR, "check_free_block_lists#1: WRONG! invalid list size at level#%d. Expected = %d, Actual = %d\n", l, expectedSizes[l], LIST_SIZE(&freeBlockLists[l]));
			isCorrect = 0;
			break;
		}
		struct BlockMetaData* curBlk ;
		LIST_FOREACH(curBlk, &freeBlockLists[l], BlockMetaData)
		{
			//Check meta data of all blocks at current level
			if (curBlk->is_free != 1 || curBlk->level != l)
			{
				cprintf_colored(TEXT_TESTERR_CLR, "check_free_block_lists#2: WRONG! invalid block meta data at level#%d. is_free = %d, level = %d\n", l, curBlk->is_free, curBlk->level);
				isCorrect = 0;
				break;
			}
			//Check VAs of blocks at current level
			bool isBlkCorrect = 0;
			for (int i = 0; i < numOfPages; ++i)
			{
				uint32 va = (uint32)pagesVAs[i];
				if (ROUNDDOWN((uint32)curBlk, PAGE_SIZE) == va)
				{
					isBlkCorrect = 1;
					break;
				}
			}
			if (!isBlkCorrect)
			{
				cprintf_colored(TEXT_TESTERR_CLR, "check_free_block_lists#3: WRONG! invalid block va at level#%d. actual = %x\n", l, curBlk);
				isCorrect = 0;
				break;
			}
		}
	}
	return isCorrect;
}



extern uint32* ptr_page_directory;
extern uint32 sys_calculate_free_frames() ;

void initialize_buddy_system(uint32 numOfPages)
{
	buddy_init();
	uint32 va = KERNEL_HEAP_START + PAGE_SIZE;
	struct FrameInfo* ptr_fi ;
	for (int i = 0; i < numOfPages; ++i)
	{
		unmap_frame(ptr_page_directory, va);
		int ret = allocate_frame(&ptr_fi);
		ret = map_frame(ptr_page_directory, ptr_fi, va, PERM_WRITEABLE);
		memset((void*)va, 0, PAGE_SIZE);
		struct BlockMetaData* left = (struct BlockMetaData*) va; left->is_free = 1; left->level = 11;
		struct BlockMetaData* right = (struct BlockMetaData*) (va + 2*kilo); right->is_free = 1; right->level = 11;
		LIST_INSERT_TAIL(&freeBlockLists[BUDDY_UPPER_LEVEL], left);
		LIST_INSERT_TAIL(&freeBlockLists[BUDDY_UPPER_LEVEL], right);
		va += PAGE_SIZE;
	}
}
/***********************************************************************************************************************/

void test_initialize_dynamic_allocator()
{
	panic("not implemented");
}

int test_initial_alloc()
{
	uint32 eval = 0;
	//Initialize Buddy System with two pages at beginning of KERN HEAP
	initialize_buddy_system(2);

	//Allocate 300B Block
	cprintf_colored(TEXT_cyan,"\n	1. Allocate 300B\n");
	int is_correct = 3;
	{
		uint32 size = 300;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + PAGE_SIZE + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 1; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 1; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 3;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Allocate 2000B Block
	cprintf_colored(TEXT_cyan,"\n	2. Allocate 2000B\n");
	is_correct = 3;
	{
		uint32 size = 2000;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + PAGE_SIZE + PAGE_SIZE/2  + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 1; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 0; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 2;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Allocate 1100B Block
	cprintf_colored(TEXT_cyan,"\n	3. Allocate 1100B\n");
	is_correct = 3;
	{
		uint32 size = 1100;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + 2*PAGE_SIZE  + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + 2*PAGE_SIZE;
		uint32 numOfBlocks = 2;
		struct BlockMetaData expectedBlocks[2];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 11;
			expectedBlocks[1].is_free = 1; expectedBlocks[1].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Allocate 1 KB Block
	cprintf_colored(TEXT_cyan,"\n	4. Allocate 1 KB\n");
	is_correct = 3;
	{
		uint32 size = 1024;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + 2*PAGE_SIZE + PAGE_SIZE/2 + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + 2*PAGE_SIZE;
		uint32 numOfBlocks = 2;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 11;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[1] = {KERNEL_HEAP_START + PAGE_SIZE};
		uint32 numOfPages = 1;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 0;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Allocate 4B Block
	cprintf_colored(TEXT_cyan,"\n	5. Allocate 4B\n");
	is_correct = 3;
	{
		uint32 size = 4;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + PAGE_SIZE + 512 + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 9;
		struct BlockMetaData expectedBlocks[9];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 4;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 4;
			expectedBlocks[3].is_free = 1; expectedBlocks[3].level = 5;
			expectedBlocks[4].is_free = 1; expectedBlocks[4].level = 6;
			expectedBlocks[5].is_free = 1; expectedBlocks[5].level = 7;
			expectedBlocks[6].is_free = 1; expectedBlocks[6].level = 8;
			expectedBlocks[7].is_free = 1; expectedBlocks[7].level = 10;
			expectedBlocks[8].is_free = 0; expectedBlocks[8].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[1] = {KERNEL_HEAP_START + PAGE_SIZE};
		uint32 numOfPages = 1;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 1;
			expectedSizes[5] = 1;
			expectedSizes[6] = 1;
			expectedSizes[7] = 1;
			expectedSizes[8] = 1;
			expectedSizes[9] = 0;
			expectedSizes[10]= 1;
			expectedSizes[11]= 0;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	return eval;
}

void test_alloc_block()
{
	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run ANYTHING before or after it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");

	int eval = 0;
	bool is_correct;
	uint32 actualSize = 0;

	int origFreeFrames = sys_calculate_free_frames();

	cprintf_colored(TEXT_cyan,"\nAllocate Some Blocks of Different Sizes [75%]\n");
	eval = test_initial_alloc();

	//Allocate ALL Remaining Blocks
	cprintf_colored(TEXT_cyan,"\nAllocate ALL Remaining Blocks using 4B Size [25%]\n");
	uint32 size = 4;
	void *va ;
	is_correct = 5;
	{
		int numOfRem4BBlocks = (kilo + kilo/2) / 16 - 1 ; //95 blocks
		for (int i = 1; i <= numOfRem4BBlocks; ++i)
		{
			va = alloc_block(size);
			//1. Check Return Address
			uint32 expectedVA = KERNEL_HEAP_START + PAGE_SIZE + 512 + 16 * i + META_DATA_SIZE;
			if ((uint32)va != expectedVA)
			{
				cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block number #%d. Expected = %x, Actual = %x\n", size, i, expectedVA, va);
				is_correct -= 2;
				break;
			}
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 3 + numOfRem4BBlocks;
		struct BlockMetaData expectedBlocks[2 + 96];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			for (int i = 0; i <= numOfRem4BBlocks; ++i)
			{
				expectedBlocks[i+1].is_free = 0; expectedBlocks[i+1].level = 4;
			}
			expectedBlocks[numOfRem4BBlocks+2].is_free = 0; expectedBlocks[numOfRem4BBlocks+2].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct-= 2;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[1] = {KERNEL_HEAP_START + PAGE_SIZE};
		uint32 numOfPages = 1;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};

		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Check number of allocated frames
	is_correct = 1;
	int freeFramesAfter = sys_calculate_free_frames();
	if (origFreeFrames - freeFramesAfter != 2) /*Only 2 pages are allocated at the begin of KHEAP*/
	{
		is_correct = 0;
		cprintf_colored(TEXT_TESTERR_CLR, "WRONG! number of allocated frames is not correct. Actual: %d, Expected: %d\n",origFreeFrames - freeFramesAfter, 2);
	}
	eval -= (1 - is_correct) * 10;
	eval = MAX(0, eval);
	cprintf_colored(TEXT_light_green, "test alloc_block Evaluation = %d%\n", eval);
	return ;
}

void test_free_block()
{
	cprintf_colored(TEXT_light_cyan, "===========================================================\n") ;
	cprintf_colored(TEXT_light_cyan, "NOTE: THIS TEST IS DEPEND ON BOTH ALLOCATE & FREE FUNCTIONS\n") ;
	cprintf_colored(TEXT_light_cyan, "===========================================================\n") ;

	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run ANYTHING before or after it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");

	int eval = 0;
	bool is_correct;
	uint32 actualSize = 0;

	int origFreeFrames = sys_calculate_free_frames();

	cprintf_colored(TEXT_cyan,"\nAllocate Some Blocks of Different Sizes [PRE-REQUISITE]\n");
	eval = test_initial_alloc();
	if (eval != 75)
	{
		panic("initial allocations are not correct!\nplease make sure the the alloc_block() test is correct before testing the free_block()\n");
	}

	eval = 0;
	//Free 1100B Block
	cprintf_colored(TEXT_cyan,"\n1. Free 1100B [No Merge] [15%]\n");
	is_correct = 3;
	{
		void *va = startVAsInit[2];
		uint32 size = get_block_size(va);
		free_block(va);
		//1. Check Size
		uint32 expectedSize = BLOCK_SIZE(11);
		if ((uint32)size != expectedSize)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Size of %dB block. Expected = %d, Actual = %d\n", size, expectedSize, size);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + 2*PAGE_SIZE;
		uint32 numOfBlocks = 2;
		struct BlockMetaData expectedBlocks[2];
		{
			expectedBlocks[0].is_free = 1; expectedBlocks[0].level = 11;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after freeing %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 1;
			expectedSizes[5] = 1;
			expectedSizes[6] = 1;
			expectedSizes[7] = 1;
			expectedSizes[8] = 1;
			expectedSizes[9] = 0;
			expectedSizes[10]= 1;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after freeing %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Free 4B Block
	cprintf_colored(TEXT_cyan,"\n2. Free 4B [Merge till 512B] [25%]\n");
	is_correct = 5;
	{
		void *va = startVAsInit[4];
		uint32 size = get_block_size(va);
		free_block(va);
		//1. Check Size
		uint32 expectedSize = BLOCK_SIZE(4);
		if ((uint32)size != expectedSize)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Size of %dB block. Expected = %d, Actual = %d\n", size, expectedSize, size);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 1; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 0; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after freeing %dB block.\n", size);
			is_correct-=2;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after freeing %dB block.\n", size);
			is_correct-=2;
		}
	}
	eval += is_correct * 5;

	//Allocate 502B Block
	cprintf_colored(TEXT_cyan,"\n3. Allocate 502B [10%]\n");
	is_correct = 2;
	{
		uint32 size = 502;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		//1. Check Return Address
		uint32 expectedVA = KERNEL_HEAP_START + PAGE_SIZE + 512 + META_DATA_SIZE;
		if ((uint32)va != expectedVA)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG VA after allocating %dB block. Expected = %x, Actual = %x\n", size, expectedVA, va);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 0; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 0; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 0;
			expectedSizes[10]= 1;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
		}

		if (!correct1 || !correct2)
		{
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Free 300B Block
	cprintf_colored(TEXT_cyan,"\n4. Free 300B [No Merge] [10%]\n");
	is_correct = 2;
	{
		void *va = startVAsInit[0];
		uint32 size = get_block_size(va);
		free_block(va);
		//1. Check Size
		uint32 expectedSize = BLOCK_SIZE(9);
		if ((uint32)size != expectedSize)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Size of %dB block. Expected = %d, Actual = %d\n", size, expectedSize, size);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 1; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 0; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after freeing %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after freeing %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Free 2000B Block
	cprintf_colored(TEXT_cyan,"\n5. Free 2000B [No Merge] [10%]\n");
	is_correct = 2;
	{
		void *va = startVAsInit[1];
		uint32 size = get_block_size(va);
		free_block(va);
		//1. Check Size
		uint32 expectedSize = BLOCK_SIZE(11);
		if ((uint32)size != expectedSize)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Size of %dB block. Expected = %d, Actual = %d\n", size, expectedSize, size);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 1; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 1; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after freeing %dB block.\n", size);
			is_correct--;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[2] = {KERNEL_HEAP_START + PAGE_SIZE, KERNEL_HEAP_START + 2*PAGE_SIZE};
		uint32 numOfPages = 2;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 2;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after freeing %dB block.\n", size);
			is_correct--;
		}
	}
	eval += is_correct * 5;

	//Allocate 2 * 1 KB Block
	cprintf_colored(TEXT_cyan,"\n6. Allocate TWO 1KB's [10%]\n");
	is_correct = 2;
	{
		uint32 size = kilo;
		void *va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;
		va = alloc_block(size);
		startVAsInit[initBlkCnt++] = va;

		//1. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 4;
		struct BlockMetaData expectedBlocks[4];
		{
			expectedBlocks[0].is_free = 1; expectedBlocks[0].level = 9;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 9;
			expectedBlocks[2].is_free = 1; expectedBlocks[2].level = 10;
			expectedBlocks[3].is_free = 0; expectedBlocks[3].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;

		pageVA = (void*)KERNEL_HEAP_START + 2*PAGE_SIZE;
		numOfBlocks = 2;
		struct BlockMetaData expectedBlocks2[2];
		{
			expectedBlocks2[0].is_free = 0; expectedBlocks2[0].level = 11;
			expectedBlocks2[1].is_free = 0; expectedBlocks2[1].level = 11;
		}
		correct1 &= check_blocks_in_page(pageVA, expectedBlocks2, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after allocating %dB block.\n", size);
			is_correct--;
		}

		//2. Check FreeBlockLists
		uint32 pagesVAs[1] = {KERNEL_HEAP_START + PAGE_SIZE};
		uint32 numOfPages = 1;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 1;
			expectedSizes[10]= 1;
			expectedSizes[11]= 0;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after allocating %dB block.\n", size);
			is_correct--;
		}

	}
	eval += is_correct * 5;

	//Free 502B Block
	cprintf_colored(TEXT_cyan,"\n7. Free 502B [Merge till 2 KB] [20%]\n");
	is_correct = 4;
	{
		void *va = startVAsInit[5];
		uint32 size = get_block_size(va);
		free_block(va);
		//1. Check Size
		uint32 expectedSize = BLOCK_SIZE(9);
		if ((uint32)size != expectedSize)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Size of %dB block. Expected = %d, Actual = %d\n", size, expectedSize, size);
			is_correct--;
		}

		//2. Check Blocks in Page
		void *pageVA = (void*)KERNEL_HEAP_START + PAGE_SIZE;
		uint32 numOfBlocks = 2;
		struct BlockMetaData expectedBlocks[2];
		{
			expectedBlocks[0].is_free = 1; expectedBlocks[0].level = 11;
			expectedBlocks[1].is_free = 0; expectedBlocks[1].level = 11;
		}
		int correct1 = check_blocks_in_page(pageVA, expectedBlocks, numOfBlocks) ;
		if (!correct1)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Page Content after freeing %dB block.\n", size);
			is_correct-=2;
		}

		//3. Check FreeBlockLists
		uint32 pagesVAs[1] = {KERNEL_HEAP_START + PAGE_SIZE};
		uint32 numOfPages = 1;
		uint32 expectedSizes[BUDDY_UPPER_LEVEL+1] = {0};
		{
			expectedSizes[4] = 0;
			expectedSizes[5] = 0;
			expectedSizes[6] = 0;
			expectedSizes[7] = 0;
			expectedSizes[8] = 0;
			expectedSizes[9] = 0;
			expectedSizes[10]= 0;
			expectedSizes[11]= 1;
		}
		int correct2 = check_free_block_lists(expectedSizes, pagesVAs, numOfPages);
		if (!correct2)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "WRONG Content of FreeBlockLists after freeing %dB block.\n", size);
			is_correct-=2;
		}
	}
	eval += is_correct * 5;

	cprintf_colored(TEXT_light_green, "test free_block completed. Evaluation = %d%\n", eval);

}

void test_realloc_block()
{
	panic("unseen test");

#if USE_KHEAP
	panic("test_realloc_block_COMPLETE: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return;
#endif
}


/********************Helper Functions***************************/
