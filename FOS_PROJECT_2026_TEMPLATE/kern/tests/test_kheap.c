#include "test_kheap.h"

#include <inc/memlayout.h>
#include <inc/queue.h>
#include <inc/dynamic_allocator.h>
#include <kern/cpu/sched.h>
#include <kern/disk/pagefile_manager.h>
#include "../mem/kheap.h"
#include "../mem/memory_manager.h"


/**********************************************************************************************/
/************************ GLOBAL DATA & FUNCTIONS FOR ALL TESTS *******************************/
/**********************************************************************************************/

#define Mega  (1024*1024)
#define kilo (1024)

#define INITIAL_PAGE_ALLOCATIONS (0)
#define ACTUAL_PAGE_ALLOC_START ((KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE) + INITIAL_PAGE_ALLOCATIONS)

extern uint32 sys_calculate_free_frames() ;
extern void sys_bypassPageFault(uint8);
extern uint32 sys_rcr2();
extern int execute_command(char *command_string);
extern char end_of_kernel[];
extern int CB(uint32 *ptr_dir, uint32 va, int bn);


/*KMALLOC*/
int test_kmalloc_FF_page();
int test_kmalloc_NF_page();
int test_kmalloc_BF_page();
int test_kmalloc_WF_page();
int test_kmalloc_CF_page();

int test_kmalloc_FF_block();
int test_kmalloc_NF_block();
int test_kmalloc_BF_block();
int test_kmalloc_WF_block();
int test_kmalloc_CF_block();

int test_kmalloc_FF_both();
int test_kmalloc_NF_both();
int test_kmalloc_BF_both();
int test_kmalloc_WF_both();
int test_kmalloc_CF_both();

/*KFREE*/
int test_kfree_FF_page();
int test_kfree_NF_page();
int test_kfree_BF_page();
int test_kfree_WF_page();
int test_kfree_CF_page();

int test_kfree_FF_block();
int test_kfree_NF_block();
int test_kfree_BF_block();
int test_kfree_WF_block();
int test_kfree_CF_block();

int test_kfree_FF_both();
int test_kfree_NF_both();
int test_kfree_BF_both();
int test_kfree_WF_both();
int test_kfree_CF_both();

/*KREALLOC*/
int test_krealloc_FF_page();
int test_krealloc_NF_page();
int test_krealloc_BF_page();
int test_krealloc_WF_page();
int test_krealloc_CF_page();

int test_krealloc_FF_block();
int test_krealloc_NF_block();
int test_krealloc_BF_block();
int test_krealloc_WF_block();
int test_krealloc_CF_block();

int test_krealloc_FF_both();
int test_krealloc_NF_both();
int test_krealloc_BF_both();
int test_krealloc_WF_both();
int test_krealloc_CF_both();

/*FAST PAGE ALLOCATOR*/
int test_fast_FF();
int test_fast_NF();
int test_fast_BF();
int test_fast_WF();
int test_fast_CF(uint32 allocSize);

int test_kmalloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_CF_both();
	}
	return 0;
}

int test_kfree(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_CF_both();
	}
	return 0;
}
int test_krealloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_CF_both();
	}
	return 0;
}

void* ptr_fast_allocations[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE] = {0};
int test_fast_page_alloc()
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		test_fast_FF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		test_fast_NF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		test_fast_BF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		test_fast_WF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test with large allocation size
		uint32 allocSize = 16*kilo;
		//uint32 allocSize = 64*kilo;
		cprintf_colored(TEXT_green ,"\nTEST FAST ALLOCATION WITH ALLOCATION SIZES OF %dKB\n\n", allocSize/kilo);
		test_fast_CF(allocSize);

	}
	return 0;
}


/**********************************************************************************************/
/*************************** INITIAL ALLOCATION FOR ALL TESTS *********************************/
/**********************************************************************************************/
#define MAX_NUM_OF_ALLOCS 20
void* ptr_allocations[MAX_NUM_OF_ALLOCS] = {0};
int lastIndices[MAX_NUM_OF_ALLOCS] = {0};
uint32 requestedSizes[MAX_NUM_OF_ALLOCS] = {0};
uint32 totalRequestedSize ;
bool allocSpaceInPageAlloc(int allocIndex, uint32 size, bool fill)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int freeDiskFrames = (int)pf_calculate_free_frames() ;
	requestedSizes[allocIndex] = size ;
	uint32 expectedNumOfFrames = ROUNDUP(requestedSizes[allocIndex], PAGE_SIZE) / PAGE_SIZE ;
	{
		ptr_allocations[allocIndex] = kmalloc(requestedSizes[allocIndex]);
	}
	if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
	if ((freeFrames - sys_calculate_free_frames()) < expectedNumOfFrames) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	lastIndices[allocIndex] = (size)/sizeof(char) - 1;
	if (fill)
	{
		char* ptr = (char*)ptr_allocations[allocIndex];
		for (int i = 0; i < lastIndices[allocIndex]; ++i)
		{
			ptr[i] = allocIndex ;
		}
	}
	return correct;

}

bool freeSpaceInPageAlloc(int allocIndex)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int freeDiskFrames = (int)pf_calculate_free_frames() ;
	uint32 expectedNumOfFrames = ROUNDUP(requestedSizes[allocIndex], PAGE_SIZE) / PAGE_SIZE ;
	{
		kfree(ptr_allocations[allocIndex]);
	}
	if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf("%d.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
	if ((sys_calculate_free_frames() - freeFrames) < expectedNumOfFrames) { correct = 0; cprintf("$d.2 Wrong kfree: pages in memory are not freed correctly\n", allocIndex); }
	return correct;
}
int initial_page_allocations(uint32 startAddr)
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	uint32 expectedVA = startAddr;

	//malloc some spaces
	int i, freeFrames, freeDiskFrames, allocIndex ;
	uint32 size = 0;
	char* ptr;
	int sums[20] = {0};
	totalRequestedSize = 0;

	int eval = 0;
	bool correct ;

	correct = 1;
	//Create some areas in PAGE allocators
	cprintf_colored(TEXT_cyan,"\n	1.1 Create some areas in PAGE allocators\n");
	{
		//4 MB
		allocIndex = 0;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//3 MB
		allocIndex = 1;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 3*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 2;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//4 MB
		allocIndex = 3;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 4;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 3*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 5;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 2*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//1 MB
		allocIndex = 6;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 1*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 7;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 8;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

	}
	//Insufficient space
	cprintf_colored(TEXT_cyan,"\n	1.2 Insufficient Space\n");
	{
		allocIndex = 9;
		expectedVA = 0;
		freeFrames = (int)sys_calculate_free_frames() ;
		freeDiskFrames = (int)pf_calculate_free_frames() ;
		uint32 restOfKHeap = (KERNEL_HEAP_MAX - ACTUAL_PAGE_ALLOC_START) - (4*Mega+3*Mega+2*Mega+4*Mega+3*Mega+2*Mega+2*Mega) ;
		ptr_allocations[allocIndex] = kmalloc(restOfKHeap+1);
		if (ptr_allocations[allocIndex] != NULL) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Allocating insufficient space: should return NULL but it return %x\n", allocIndex, ptr_allocations[allocIndex]); }
		if (((int)pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
		if ((freeFrames - (int)sys_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	}
	if (correct)	eval+=10 ;

	return eval;
}

short* startBlockVAs[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/ DYN_ALLOC_MIN_BLOCK_SIZE] = {0} ;
uint8 levelOfEachAllocBlock[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/ DYN_ALLOC_MIN_BLOCK_SIZE] = {0} ;
#define numOfLevels (BUDDY_UPPER_LEVEL + 1)
int numOfAllocBlocksPerSize[numOfLevels] = {0};
int numOfAllocPages = 0;
int numOfAllocBlocks = 0;

int initial_block_allocations()
{
	memset(numOfAllocBlocksPerSize, 0, numOfLevels * sizeof(int)) ;
	numOfAllocPages = 0;

	int eval = 0;
	bool is_correct = 1;

	int freeFramesBefore = (int)sys_calculate_free_frames();
	int freeDiskFramesBefore = (int)pf_calculate_free_frames() ;
	uint32 currentVA = KERNEL_HEAP_START;

	void *va ;
	int idx = 0;
	//====================================================================//
	/*1: Check initial allocations (two blocks should be allocated at start-up 16B & 32B)*/
	cprintf_colored(TEXT_cyan, "\n	1.1: Check initial BLOCK allocations (two blocks should be allocated at start-up 16B & 32B)\n\n") ;
	uint32 numOfRemain32 = 0;
	uint32 numOfRemain16 = 1;
	if (LIST_SIZE(&freeBlockLists[5]) != numOfRemain32 || LIST_SIZE(&freeBlockLists[4]) != numOfRemain16)
	{
		is_correct = 0;
		cprintf_colored(TEXT_TESTERR_CLR, "initial allocation #1: unexpected list size! check boot-time allocation\n");
	}
	numOfAllocBlocksPerSize[4]++ ;
	numOfAllocBlocksPerSize[5]++ ;
	numOfAllocPages++;

	if (is_correct) eval += 10;
	//====================================================================//
	/*2: Allocate all remaining blocks from the current page*/
	{
		cprintf_colored(TEXT_cyan, "\n	1.2: Allocate all remaining blocks from the current page\n\n") ;
		is_correct = 1;
		for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
		{
			if (l == 5) continue; //skip this level since it has no free blocks due to the previous 16B & 32B alloc's
			idx++;
			int curSize = BLOCK_SIZE(l) - META_DATA_SIZE ;
			//allocate a new block with the curSize
			{
				va = kmalloc(curSize);
				startBlockVAs[idx] = va;
				levelOfEachAllocBlock[idx] = l;
			}

			//fill the entire block
			for (int i = 0; i < curSize/2; ++i)
			{
				startBlockVAs[idx][i] = idx;
			}

			numOfAllocBlocksPerSize[l]++ ;

			uint32 expectedVA = currentVA ;
			if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "initial allocation #2.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", idx, expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
			}
		}
		if (is_correct) eval += 20;
	}

	//====================================================================//
	/*3: For each possible block size, allocate all, except one, blocks from one page */
	cprintf_colored(TEXT_cyan, "	1.3: For each possible block size, allocate all, except one, blocks from one page\n\n") ;
	{
		is_correct = 1;
		for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
		{
			int curSize = BLOCK_SIZE(l)  ;
			int maxNumOfBlksAtCurPage = PAGE_SIZE / curSize;
			curSize -= META_DATA_SIZE;

			numOfAllocPages++; currentVA += PAGE_SIZE;

			for (int b = 0; b < maxNumOfBlksAtCurPage - 1; ++b)
			{
				idx++;
				//allocate a new block with the curSize
				{
					va = kmalloc(curSize);
					startBlockVAs[idx] = va;
					levelOfEachAllocBlock[idx] = l;
				}
				//fill the entire block
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = idx;
				}

				numOfAllocBlocksPerSize[l]++ ;

				//check the address
				uint32 expectedVA = currentVA ;
				if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "initial allocation #3.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", idx, expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
				}
			}

		}
		if (is_correct) eval += 30;
	}

	//====================================================================//
	/*4: Check content of each block */
	numOfAllocBlocks = idx;
	{
		cprintf_colored(TEXT_cyan, "	1.4: Check content of each block \n\n") ;
		is_correct = 1;
		for (int b = 1; b <= numOfAllocBlocks; ++b)
		{
			//check the content of the current block
			int sum = 0;
			int curSize = BLOCK_SIZE(levelOfEachAllocBlock[b]) - META_DATA_SIZE;

			for (int i = 0; i < curSize/2; ++i)
			{
				sum += startBlockVAs[b][i] ;
			}
			if (is_correct && sum != b * curSize/2)
			{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d wrong content\n", b); }
		}
		if (is_correct) eval += 20;
	}
	//====================================================================//
	/*5: Check number of allocated pages */
	cprintf_colored(TEXT_cyan, "	1.5: Check number of allocated pages \n\n") ;
	{
		is_correct = 1;
		if ((pf_calculate_free_frames() - freeDiskFramesBefore) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		//exclude the 1st two pages that are allocated to 16B & 8B during boot-time
		if ((freeFramesBefore - sys_calculate_free_frames()) != (numOfAllocPages-1) /*as 1 page already allocated at boot time*/)
		{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", numOfAllocPages,freeFramesBefore - sys_calculate_free_frames()); }
		if (is_correct)	eval += 20;
	}
	return eval;
}

/**********************************************************************************************/
/********************************** KMALLOC TESTING AREA **************************************/
/**********************************************************************************************/
int test_kmalloc_FF_page()
{
	panic("not implemented function");
}
int test_kmalloc_NF_page()
{
	panic("not implemented function");
}
int test_kmalloc_BF_page()
{
	panic("not implemented function");
}
int test_kmalloc_WF_page()
{
	panic("not implemented function");
}

int test_kmalloc_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces
	int eval = 0;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces [70%]\n");
	{
		eval = initial_page_allocations(ACTUAL_PAGE_ALLOC_START);
		eval = ROUNDUP(eval * 70 / 100, 5); //rescale
	}

	//2. Check BREAK
	int correct = 1;
	cprintf_colored(TEXT_cyan,"\n2. Check Page Allocator BREAK [10%]\n");
	{
		uint32 allocSizes = 0;
		for (int i = 0; i < 9; ++i)
		{
			allocSizes += ROUNDUP(requestedSizes[i], PAGE_SIZE);
		}
		uint32 expectedVA = ACTUAL_PAGE_ALLOC_START + allocSizes;
		if(kheapPageAllocBreak != expectedVA) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedVA, kheapPageAllocBreak);}
	}
	if (correct) eval += 10;
	correct = 1;

	//3. Check Permissions
	cprintf_colored(TEXT_cyan,"\n3. Check permissions of allocated spaces in PAGE ALLOCATOR [10%]\n");
	{
		uint32 lastAllocAddress = (uint32)ptr_allocations[8] + 2*Mega ;
		uint32 va;
		for (va = ACTUAL_PAGE_ALLOC_START; va < lastAllocAddress; va+=PAGE_SIZE)
		{
			unsigned int * table;
			get_page_table(ptr_page_directory, va, &table);
			uint32 perm = table[PTX(va)] & 0xFFF;
			if ((perm & PERM_USER) == PERM_USER)
			{
				if (correct)
				{
					correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong permissions: pages should be mapped with Supervisor permission only\n");
				}
			}
		}
	}
	if (correct) eval += 10;
	correct = 1;

	//4. Check Content
	uint32 sums[MAX_NUM_OF_ALLOCS] = {0};
	cprintf_colored(TEXT_cyan,"\n4. Check Content [10%]\n");
	{
		for (int i = 0; i < 9; ++i)
		{
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 10;
	correct = 1;

	cprintf_colored(TEXT_light_green,"\nTest kmalloc Page Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kmalloc_FF_block()
{
	panic("not implemented function");
}
int test_kmalloc_NF_block()
{
	panic("not implemented function");
}
int test_kmalloc_BF_block()
{
	panic("not implemented function");
}
int test_kmalloc_WF_block()
{
	panic("not implemented function");
}
extern void PrintFreeBlockLists();
int test_kmalloc_CF_block()
{
	//panic("UPDATE IS REQUIRED");

	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces
	int eval = 0;
	cprintf_colored(TEXT_cyan,"1. Alloc some spaces [50%]\n");
	{
		eval = initial_block_allocations();
		eval = ROUNDUP(eval * 50 / 100, 5); //rescale
	}
	//2. Fill-up the remaining pages using the 2KB block
	void *va;
	int is_correct;
	cprintf_colored(TEXT_cyan,"2. Fill-up the remaining pages using the 2KB block [15%]\n");
	{
		is_correct = 1;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		int requiredNumOf2KBBlks = ((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE - numOfAllocPages) * 2;
		int curIndex = BUDDY_UPPER_LEVEL;
		int curSize = DYN_ALLOC_MAX_BLOCK_SIZE - META_DATA_SIZE;
		//allocate the remaining 2KB block from last level
		{
			va = kmalloc(curSize);
		}
		numOfAllocBlocksPerSize[curIndex]++ ;
		//Fill-up the remaining pages with 2KB alloc's

		{
			//kmalloc((KERNEL_HEAP_MAX-KERNEL_HEAP_START) - numOfAllocPages*PAGE_SIZE);
		}
		uint32 currentVA = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE ;
		int progressBarStep = requiredNumOf2KBBlks / 20 ;
		//cprintf("requiredNumOf2KBBlks = %d\n20#: ", requiredNumOf2KBBlks);
		for (int i = 1; i <= requiredNumOf2KBBlks; ++i)
		{
			if (i % progressBarStep == 0)
				cprintf("#");
			//allocate a new block with the curSize
			{
				va = kmalloc(curSize);
			}
			numOfAllocBlocksPerSize[curIndex]++ ;

			uint32 expectedVA = currentVA ;
			if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #5.%d: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n", i, expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
			}
			if (i%2 == 0)
			{
				numOfAllocPages++;
				currentVA += PAGE_SIZE;
			}
		}
		cprintf("\n");
		int expectedNumOfAllocPages = requiredNumOf2KBBlks / 2;
		if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		if ((freeFrames - sys_calculate_free_frames()) != expectedNumOfAllocPages) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", expectedNumOfAllocPages, freeFrames - sys_calculate_free_frames()); }

		if (is_correct) eval += 10;
		is_correct = 1;
	}

	//3: Allocate the single remaining block from each level*/
	cprintf_colored(TEXT_cyan, "\n3: Allocate the single remaining block from each level\n\n") ;
	{
		is_correct = 1;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		uint32 expectedVA = KERNEL_HEAP_START ;
		for (int l = BUDDY_LOWER_LEVEL; l < BUDDY_UPPER_LEVEL; ++l) //block from last level already allocated in prev step
		{
			int curSize = BLOCK_SIZE(l) - META_DATA_SIZE ;
			//allocate a new block with the curSize
			{
				va = kmalloc(curSize);
			}

			numOfAllocBlocksPerSize[l]++ ;

			expectedVA += PAGE_SIZE;
			if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #6: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n",  expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
			}
		}
		int expectedNumOfAllocPages = 0;
		if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		if ((freeFrames - sys_calculate_free_frames()) != expectedNumOfAllocPages) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", expectedNumOfAllocPages, freeFrames - sys_calculate_free_frames()); }
		if (is_correct) eval += 10;
	}

	//4. Run out-of-heap memory
	cprintf_colored(TEXT_cyan,"4. Run out-of-heap memory [15%]\n");
	{
		is_correct = 1;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
		{
			int curSize = BLOCK_SIZE(l) - META_DATA_SIZE ;
			//allocate a new block with the curSize
			{
				va = kmalloc(curSize);
			}
			//cprintf("va = kmalloc(%d) => %x\n", curSize, va);
			uint32 expectedVA = 0;
			if (is_correct && ROUNDDOWN((uint32)va, PAGE_SIZE) != expectedVA)
			{
				is_correct = 0;
				cprintf_colored(TEXT_TESTERR_CLR, "Block Alloc #7: WRONG! VA is not correct. Expected VA = %x, Actual VA = %x\n",  expectedVA, ROUNDDOWN((uint32)va, PAGE_SIZE));
			}
		}

		int expectedNumOfAllocPages = 0;
		if ((pf_calculate_free_frames() - freeDiskFrames) != 0) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
		if ((freeFrames - sys_calculate_free_frames()) != expectedNumOfAllocPages) { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Wrong allocation: invalid number of allocated pages! Expected = %d, Actual = %d\n", expectedNumOfAllocPages, freeFrames - sys_calculate_free_frames()); }
		if (is_correct) eval += 20;
	}

	//5. Check Permissions
	cprintf_colored(TEXT_cyan,"\n5. Check permissions of allocated spaces in BLOCK ALLOCATOR [10%]\n");
	{
		is_correct = 1;
		uint32 lastAllocAddress = (uint32)ptr_allocations[8] + 2*Mega ;
		uint64 va;
		for (va = KERNEL_HEAP_START; va < KERNEL_HEAP_MAX; va+=PAGE_SIZE)
		{
			unsigned int * table;
			get_page_table(ptr_page_directory, (uint32)va, &table);
			uint32 perm = table[PTX(va)] & 0xFFF;
			if ((perm & PERM_USER) == PERM_USER || (perm & PERM_WRITEABLE) != PERM_WRITEABLE || (perm & PERM_UHPAGE) == PERM_UHPAGE)
			{
				if (is_correct)
				{
					is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"Block Alloc #8: Wrong permissions: pages should be mapped with Supervisor permission only\n");
				}
			}
		}
		if (is_correct) eval += 10;
	}

	cprintf_colored(TEXT_light_green,"\nTest kmalloc Block Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kmalloc_FF_both()
{
	panic("not implemented function");
}
int test_kmalloc_NF_both()
{
	panic("not implemented function");
}
int test_kmalloc_BF_both()
{
	panic("not implemented function");
}
int test_kmalloc_WF_both()
{
	panic("not implemented function");
}
int test_kmalloc_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/*********************************** KFREE TESTING AREA ***************************************/
/**********************************************************************************************/
int test_kfree_FF_page()
{
	panic("not implemented function");
}
int test_kfree_NF_page()
{
	panic("not implemented function");
}
int test_kfree_BF_page()
{
	panic("not implemented function");
}
int test_kfree_WF_page()
{
	panic("not implemented function");
}

int test_kfree_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	//1. Alloc some spaces in PAGE allocator
	int correct = 1;
	int eval;
	cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in PAGE allocator\n");
	{
		eval = initial_page_allocations(ACTUAL_PAGE_ALLOC_START);
		if (eval != 100)
		{
			cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kfree\n");
			return 0;
		}
	}
	eval = 0;
	//2. Free some allocations to create initial holes
	cprintf_colored(TEXT_cyan,"\n2. Free some allocations to create initial holes [5%]\n");
	correct = 1;
	{
		//3 MB Hole
		correct = freeSpaceInPageAlloc(1);

		//2nd 4 MB Hole
		correct = freeSpaceInPageAlloc(3);

		//2nd 1 MB Hole
		correct = freeSpaceInPageAlloc(5);

		//2nd 2 MB Hole
		correct = freeSpaceInPageAlloc(7);
	}
	if (correct) eval += 5;
	correct = 1;

	//3. Check content of un-freed spaces
	uint32 sums[MAX_NUM_OF_ALLOCS] = {0};
	cprintf_colored(TEXT_cyan,"\n3. Check content of un-freed spaces [5%]\n");
	{
		for (int i = 0; i < 9; ++i)
		{
			//skip the freed spaces
			if (i == 1 || i == 3 || i == 5 || i == 7)
				continue;
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 5;
	correct = 1;

	//4. Check BREAK
	correct = 1;
	uint32 expectedBreak = 0;
	cprintf_colored(TEXT_cyan,"\n4. Check BREAK [5%]\n");
	{
		uint32 allocSizes = 0;
		for (int i = 0; i < 9; ++i)
		{
			allocSizes += ROUNDUP(requestedSizes[i], PAGE_SIZE);
		}
		expectedBreak = ACTUAL_PAGE_ALLOC_START + allocSizes;
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}
	}
	if (correct) eval += 5;
	correct = 1;

	//5. Allocate after kfree [Test CUSTOM FIT]
	uint32 allocIndex,expectedVA, size = 0;
	cprintf_colored(TEXT_cyan,"\n5. Allocate after kfree [Test CUSTOM FIT] [30%]\n");
	{
		//1 MB [EXACT FIT in 1MB Hole (alloc#5)]
		allocIndex = 10;
		size = 1*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] ; //Address of 1MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//1MB + 4KB [WORST FIT in 4MB Hole (alloc#3)]
		allocIndex = 11;
		size = 1*Mega + 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] ; //Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3MB - 4KB [EXACT FIT in remaining area of 4MB Hole (alloc#3)]
		allocIndex = 12;
		size = 3*Mega - 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] + 1*Mega + 4*kilo; //1MB.4KB after the Start Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//1.5 MB [WORST FIT in 3MB Hole (alloc#1)]
		allocIndex = 13;
		size = 1*Mega + Mega/2;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[1] ; //Address of 3MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//2.5 MB [EXTEND THE BREAK]
		allocIndex = 14;
		size = 2*Mega + Mega/2;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = expectedBreak ;
		expectedBreak += ROUNDUP(size, PAGE_SIZE);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}

		//Insufficient space
		allocIndex = 15;
		expectedVA = 0;
		int freeFrames = (int)sys_calculate_free_frames() ;
		int freeDiskFrames = (int)pf_calculate_free_frames() ;
		uint32 restOfKHeap = (KERNEL_HEAP_MAX - ACTUAL_PAGE_ALLOC_START) - expectedBreak ;
		ptr_allocations[allocIndex] = kmalloc(restOfKHeap+1);
		if (ptr_allocations[allocIndex] != NULL) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.1 Allocating insufficient space: should return NULL\n", allocIndex); }
		if (((int)pf_calculate_free_frames() - freeDiskFrames) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
		if ((freeFrames - (int)sys_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	}
	if (correct) eval+=30;
	correct = 1;

	//6. Check content of newly allocated  spaces
	cprintf_colored(TEXT_cyan,"\n6. Check content of newly allocated spaces [5%]\n");
	{
		for (int i = 10; i < 15; ++i)
		{
			char* ptr = (char*)ptr_allocations[i];
			for (int j = 0; j < lastIndices[i]; ++j)
			{
				sums[i] += ptr[j] ;
			}
			if (sums[i] != i*lastIndices[i])	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"invalid content\n"); }
		}
	}
	if (correct) eval += 5;
	correct = 1;

	//7. Free some allocations to create MERGED holes
	correct = 1;
	cprintf_colored(TEXT_cyan,"\n7. Free some allocations to create MERGED holes [10%]\n");
	{
		//Free new 3MB-4KB allocation inside the 4MB Hole
		correct = freeSpaceInPageAlloc(12);

		//Free new 1MB+4KB allocation at the beginning of the 4MB Hole (should be MERGED with next 3MB) => 4MB HOLE
		//cprintf_colored(TEXTBG_red, "SHOULD BE MERGE WITH NEXT\n");
		correct = freeSpaceInPageAlloc(11);

		//Free new 1.5MB allocation at the beginning of the 3MB Hole (should be MERGED with next 1.5MB) => 3MB HOLE
		//cprintf_colored(TEXTBG_red, "SHOULD BE MERGE WITH NEXT\n");
		correct = freeSpaceInPageAlloc(13);

		//Free new 1MB allocation at the 1MB Hole (NO MERGED)
		//cprintf_colored(TEXTBG_red, "SHOULD BE NO MERGE (va = %x)\n", ptr_allocations[10]);
		correct = freeSpaceInPageAlloc(10);

		//Free original 3rd 1MB allocation (should be MERGED with next 2MB hole and the prev 1MB hole) => 4MB HOLE
		//cprintf_colored(TEXTBG_red, "SHOULD BE MERGE WITH NEXT & PREV (va = %x)\n", ptr_allocations[6]);
		correct = freeSpaceInPageAlloc(6);

		//Free original last 2MB allocation (should be MERGED with the prev 4MB created hole) => 6MB HOLE
		//cprintf_colored(TEXTBG_red, "SHOULD BE MERGE WITH PREV\n");
		correct = freeSpaceInPageAlloc(8);
	}
	if (correct) eval += 10;
	correct = 1;

	//8. Allocate after kfree [Test CUSTOM FIT in MERGED FREE SPACES]
	cprintf_colored(TEXT_cyan,"\n8. Allocate after kfree [Test CUSTOM FIT in MERGED FREE SPACES] [20%]\n");
	{
		//3 MB [EXACT FIT in 3MB Hole]
		allocIndex = 16;
		size = 3*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[1] ; //Address of 3MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3 MB [WORST FIT in 6MB Hole]
		allocIndex = 17;
		size = 3*Mega - kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] ; //Address of 6MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3MB - 4KB [WORST FIT in 4MB Hole]
		allocIndex = 18;
		size = 3*Mega - 4*kilo;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[3] ; //Address of 4MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

		//3 MB [EXACT FIT in remaining of 6MB Hole]
		allocIndex = 19;
		size = 3*Mega;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1);
		expectedVA = (uint32)ptr_allocations[5] + 3*Mega ; //3MB after the start address of 6MB Hole
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
	}
	if (correct) eval += 20;
	correct = 1;

	//9. Free last allocations to move-down the BREAK
	correct = 1;
	cprintf_colored(TEXT_cyan,"\n9. Free last allocations to move-down the BREAK [10%]\n");
	{
		//Free last allocated 3MB
		correct = freeSpaceInPageAlloc(19);

		//Free 2.5MB allocation (should be merged with prev 3MB and the break should be moved-down)
		correct = freeSpaceInPageAlloc(14);

		expectedBreak = expectedBreak - (5*Mega + Mega/2) ;
		if(kheapPageAllocBreak != expectedBreak) {correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"BREAK value is not correct! Expected = %x, Actual = %x\n", expectedBreak, kheapPageAllocBreak);}
	}
	if (correct) eval += 10;
	correct = 1;

	//Check memory access of FREED area in PAGE allocator
	char* ptr;
	cprintf_colored(TEXT_cyan,"\n10. Check memory access of FREED area in PAGE allocator [5%]\n");
	{
		//Bypass the PAGE FAULT on <MOVB immediate, reg> instruction by setting its length
		//and continue executing the remaining code
		sys_bypassPageFault(3);

		ptr = (char *) ptr_allocations[18] + requestedSizes[18]; //begin of free 1MB after the 3rd 3MB allocation
		ptr[0] = 10;
		//cprintf("\n\ncr2 = %x, faulted addr = %x", sys_rcr2(), (uint32)&(ptr[0]));
		if (sys_rcr2() != (uint32)&(ptr[0]))
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"10 kfree: successful access to freed space!! it should not be succeeded\n"); }
		ptr[1*Mega-1] = 10;
		if (sys_rcr2() != (uint32)&(ptr[1*Mega-1]))
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"10 kfree: successful access to freed space!! it should not be succeeded\n"); }

		//set it to 0 again to cancel the bypassing option
		sys_bypassPageFault(0);
	}
	if (correct) eval += 5;
	correct = 1;

	correct = 1 ;
	//11. Check page tables
	cprintf_colored(TEXT_cyan,"\n11. check page tables [5%]\n");
	{
		long long va;
		for (va = KERNEL_HEAP_START; va < (long long)KERNEL_HEAP_MAX; va+=PTSIZE)
		{
			uint32 *ptr_table ;
			get_page_table(ptr_page_directory, (uint32)va, &ptr_table);
			if (ptr_table == NULL)
			{
				if (correct)
				{ correct = 0; cprintf("Wrong kfree: one of the kernel tables is wrongly removed! Tables should not be removed here in kfree\n"); }
			}
		}
	}
	if (correct)	eval+=5 ;

	cprintf_colored(TEXT_light_green,"\nTest kfree & CUSTOM FIT Page Alloc Completed. Evaluation = %d%\n", eval);
	return 0;
}

int test_kfree_FF_block()
{
	panic("not implemented function");
}
int test_kfree_NF_block()
{
	panic("not implemented function");
}
int test_kfree_BF_block()
{
	panic("not implemented function");
}
int test_kfree_WF_block()
{
	panic("not implemented function");
}
int test_kfree_CF_block()
{
		cprintf_colored(TEXT_yellow,"==============================================\n");
		cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
		cprintf_colored(TEXT_yellow,"==============================================\n");

		int origFreeFrames = (int)sys_calculate_free_frames();
		int eval ;

		//1. Alloc some blocks at each possible block size
		cprintf_colored(TEXT_cyan,"\n1. Alloc some blocks at each possible block size\n");
		{
			eval = initial_block_allocations();
			if (eval != 100)
			{
				cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kfree\n");
				return 0;
			}
		}

		eval = 0;
		int is_correct ;
		int curSize, idx, maxNumOfBlksAtCurPage;

		//2. Free some blocks WITH freeing their pages
		cprintf_colored(TEXT_cyan,"\n2. Free some blocks WITH freeing their pages [25%]\n");
		int expectedNumOfRemovedPages = 0;
		uint32 startOfFreedBlocks[4] = {0};
		uint32 endOfFreedBlocks[4] = {0};
		is_correct = 1;
		{
			int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
			int idx=0;
			//kfree ALL blocks in first buddy (except two blocks of boot-time)
			for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
			{
				if (l == 5) continue; //skip this level since it has no free blocks due to the previous 16B & 32B alloc's
				idx++;
				kfree(startBlockVAs[idx]);
				numOfAllocBlocksPerSize[l]--;
			}
			idx++ ;

			//kfree ALL allocated 64B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(4) - 1 + PAGE_SIZE / BLOCK_SIZE(5) - 1;
			startOfFreedBlocks[0] = idx;
			int curLevel = 6;
			for (int s = 0; s < PAGE_SIZE / 64 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[curLevel]--;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == 0);
			endOfFreedBlocks[0] = idx-1;

			//kfree ALL allocated 256B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(7) - 1 ;
			startOfFreedBlocks[2] = idx;
			curLevel = 8;
			for (int s = 0; s < PAGE_SIZE / 256 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[curLevel]--;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == 0);
			endOfFreedBlocks[2] = idx-1;

			//Check # free frames (should be changed)
			{
				expectedNumOfRemovedPages = 2; //page of 64B blocks & page of 256B blocks
				freeFramesAfter = sys_calculate_free_frames();
				if (freeFramesAfter - freeFramesBefore != expectedNumOfRemovedPages)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #2: WRONG! number of free frames is not as expected. Actual: %d, Expected: %d\n",freeFramesAfter - freeFramesBefore, expectedNumOfRemovedPages);
				}
			}
		}
		if (is_correct) eval += 25;

		//3. Allocate same blocks after free with diff. content (pages should be allocated)
		cprintf_colored(TEXT_cyan,"\n3. Allocate same blocks after free! diff. content (pages should be allocated) [15%]\n");
		is_correct = 1;
		{
			int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
			int idx = 0;
			//kmalloc ALL blocks in first buddy (except two blocks of boot-time)
			for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
			{
				if (l == 5) continue; //skip this level since it has no free blocks due to the previous 16B & 32B alloc's
				idx++;
				int curSize = BLOCK_SIZE(l) - META_DATA_SIZE ;
				//allocate a new block with the curSize
				{
					startBlockVAs[idx] = kmalloc(curSize);
					levelOfEachAllocBlock[idx] = l;
				}
				//fill the entire block with NEW values
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = 2*idx;
				}

				numOfAllocBlocksPerSize[l]++ ;
			}

			//kmalloc ALL freed 64B blocks
			int curSize = 64 - META_DATA_SIZE;
			int curLevel = 6;
			for (int idx = startOfFreedBlocks[0]; idx <= endOfFreedBlocks[0]; ++idx)
			{
				//allocate a new block with the curSize
				{
					startBlockVAs[idx] = kmalloc(curSize);
					levelOfEachAllocBlock[idx] = curLevel;
				}
				//fill the entire block
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = 2*idx;
				}
				numOfAllocBlocksPerSize[curLevel]++ ;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == PAGE_SIZE / 64 - 1 + 1/*1 of re-allocated blk from 1st buddy*/);

			//kmalloc ALL freed 256B blocks
			curSize = 256 - META_DATA_SIZE;
			curLevel = 8;
			for (int idx = startOfFreedBlocks[2]; idx <= endOfFreedBlocks[2]; ++idx)
			{
				//allocate a new block with the curSize
				{
					startBlockVAs[idx] = kmalloc(curSize);
					levelOfEachAllocBlock[idx] = curLevel;
				}
				//fill the entire block
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = 2*idx;
				}
				numOfAllocBlocksPerSize[curLevel]++ ;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == PAGE_SIZE / 256 - 1 + 1/*1 of re-allocated blk from 1st buddy*/);

			//Check # free frames (should be changed)
			{
				expectedNumOfRemovedPages = 2; //page of 64B blocks & page of 256B blocks
				freeFramesAfter = sys_calculate_free_frames();
				if (freeFramesBefore - freeFramesAfter != expectedNumOfRemovedPages)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #3: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n", freeFramesBefore - freeFramesAfter, expectedNumOfRemovedPages);
				}
			}
		}
		if (is_correct) eval += 15;

		//4. Free some blocks WITHOUT freeing their pages
		cprintf_colored(TEXT_cyan,"\n4. Free some blocks WITHOUT freeing their pages [15%]\n");
		is_correct = 1;
		{
			int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
			int idx=0;
			//kfree ALL allocated 128B blocks EXCEPT last one
			idx = endOfFreedBlocks[0] + 1;
			startOfFreedBlocks[1] = idx;
			int curLevel = 7;
			for (int s = 0; s < PAGE_SIZE / 128 - 2; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[curLevel]--;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == 1 + 1/*1 of re-allocated blk from 1st buddy*/);
			endOfFreedBlocks[1] = idx-1;

			//kfree ALL allocated 512B blocks EXCEPT last one
			idx = endOfFreedBlocks[2] + 1;
			startOfFreedBlocks[3] = idx;
			curLevel = 9;
			for (int s = 0; s < PAGE_SIZE / 512 - 2; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[curLevel]--;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == 1 + 1/*1 of re-allocated blk from 1st buddy*/);
			endOfFreedBlocks[3] = idx-1;

			//Check # free frames (should not be changed)
			{
				freeFramesAfter = sys_calculate_free_frames();
				if (freeFramesAfter != freeFramesBefore)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #4: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",freeFramesBefore - freeFramesAfter , 0);
				}
			}
		}
		if (is_correct) eval += 15;

		//5. Allocate same blocks after free with diff. content (pages should NOT be allocated)
		cprintf_colored(TEXT_cyan,"\n5. Allocate same blocks after free! diff. content (pages should NOT be allocated) [15%]\n");
		is_correct = 1;
		{
			int freeFramesAfter = 0, freeFramesBefore = sys_calculate_free_frames();
			int idx=0;

			//kmalloc ALL freed 128B blocks
			int curSize = 128 - META_DATA_SIZE;
			int curLevel = 7;
			for (int idx = startOfFreedBlocks[1]; idx <= endOfFreedBlocks[1]; ++idx)
			{
				//allocate a new block with the curSize
				{
					startBlockVAs[idx] = kmalloc(curSize);
					levelOfEachAllocBlock[idx] = curLevel;
				}
				//fill the entire block
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = 2*idx;
				}
				numOfAllocBlocksPerSize[curLevel]++ ;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == PAGE_SIZE / 128 - 1 + 1/*1 of re-allocated blk from 1st buddy*/);

			//kmalloc ALL freed 512B blocks
			curSize = 512 - META_DATA_SIZE;
			curLevel = 9;
			for (int idx = startOfFreedBlocks[3]; idx <= endOfFreedBlocks[3]; ++idx)
			{
				//allocate a new block with the curSize
				{
					startBlockVAs[idx] = kmalloc(curSize);
					levelOfEachAllocBlock[idx] = curLevel;
				}
				//fill the entire block
				for (int i = 0; i < curSize/2; ++i)
				{
					startBlockVAs[idx][i] = 2*idx;
				}

				numOfAllocBlocksPerSize[curLevel]++ ;
			}
			assert(numOfAllocBlocksPerSize[curLevel] == PAGE_SIZE / 512 - 1 + 1/*1 of re-allocated blk from 1st buddy*/);

			//Check # free frames (should not be changed)
			{
				freeFramesAfter = sys_calculate_free_frames();
				if (freeFramesAfter != freeFramesBefore)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #5: WRONG! number of allocated frames is not as expected. Actual: %d, Expected: %d\n",freeFramesBefore - freeFramesAfter , 0);
				}
			}
		}
		if (is_correct) eval += 15;

		//6. Check content of all blocks
		cprintf_colored(TEXT_cyan,"\n6. Check content of all blocks [20%]\n");
		is_correct = 1;
		{
			idx = 0;
			int mult ;
			for (int s = 1; s <= numOfAllocBlocks; ++s)
			{
				//Blocks of original value
				if ((s > (BUDDY_UPPER_LEVEL - BUDDY_LOWER_LEVEL) && s < startOfFreedBlocks[0]) /*16B & 32B blocks*/ ||
						s == endOfFreedBlocks[1] + 1 /*last block of 128B's*/ ||
						s > endOfFreedBlocks[3] /*all blocks after 512B*/)
					mult = 1;
				else
					mult = 2;

				curSize = BLOCK_SIZE(levelOfEachAllocBlock[s]) - META_DATA_SIZE;
				//check the content of the current block
				int sum = 0;
				for (int i = 0; i < curSize/2; ++i)
				{
					sum += startBlockVAs[s][i] ;
				}
				if (is_correct && sum != mult * s * curSize/2)
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR,"Free Block Alloc #6: wrong content at block# %d of level# %d. Expected = %d, Actual = %d\n", s,levelOfEachAllocBlock[s], mult * s * curSize/2, sum);
					break;
				}
			}
		}
		if (is_correct) eval += 20;

		//7. Free all blocks
		cprintf_colored(TEXT_cyan,"\n7. Free all blocks [10%]\n");
		is_correct = 1;
		{
			int freeFramesAfter = 0 ;
			for (int s = 1; s <= numOfAllocBlocks; ++s)
			{
				//Free the current block
				{
					kfree(startBlockVAs[s]);
					numOfAllocBlocksPerSize[levelOfEachAllocBlock[s]]--;
				}
			}
			//Check # free frames (should reflect the number of freed pages & non-freed ones)
			{
				freeFramesAfter = sys_calculate_free_frames();
				if (origFreeFrames - freeFramesAfter > 2) /*1 or 2 additional pages may be left due to using special data structures in Page Alloc*/
				{
					is_correct = 0;
					cprintf_colored(TEXT_TESTERR_CLR, "Free Block Alloc #7: WRONG! number of free frames is not restored correctly. Actual: %d, Expected (at least): %d\n",origFreeFrames - freeFramesAfter, 2);
				}
			}
		}
		if (is_correct) eval += 10;

		cprintf_colored(TEXT_light_green,"\nTest kfree Block Alloc Completed. Evaluation = %d%\n", eval);
		return 0;
}

int test_kfree_FF_both()
{
	panic("not implemented function");
}
int test_kfree_NF_both()
{
	panic("not implemented function");
}
int test_kfree_BF_both()
{
	panic("not implemented function");
}
int test_kfree_WF_both()
{
	panic("not implemented function");
}
int test_kfree_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/****************************** ADDRESS CONVERSION TEST AREA **********************************/
/**********************************************************************************************/
inline bool isVAInsideFreedAreas(uint32 va, uint32 *startOfFreedAreas, uint32 *endOfFreedAreas, int numOfFreedAreas)
{
	for (int i = 0; i < numOfFreedAreas; ++i)
	{
		if (va >= startOfFreedAreas[i] && va <= endOfFreedAreas[i])
			return 1;
	}
	return 0;
}
uint32 allPAs[(128*Mega)/PAGE_SIZE] ;

int test_kheap_phys_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	int correct = 1;
	int eval = 0;
	uint32 startOfPageAlloc = 0;
	uint32 expected;

	/*FIRST: BLOCK ALLOCATOR*/
	{
		//1. Alloc some spaces in the allocator
		cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in BLOCK allocator\n");
		{
			uint32 res = initial_block_allocations();
			startOfPageAlloc = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE;
			if (res != 100)
			{
				cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
				return 0;
			}
		}
		//2. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc only
		cprintf_colored(TEXT_cyan,"\n2. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc only [20%]\n");
		correct = 1 ;
		{
			int i;
			uint32 va, pa;
			for (i = 1; i <= numOfAllocBlocks; i++)
			{
				va = (uint32)startBlockVAs[i];
				pa = kheap_physical_address(va);
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("2.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				if (((ptr_table[PTX(va)] & 0xFFFFF000)+(va & 0x00000FFF))!= pa)
				{
					//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
					if (correct)
					{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"2.2 Wrong kheap_physical_address\n"); }
				}
			}
		}
		if (correct)	eval+=20 ;

		//3. kfree some of the allocated spaces in BLOCK allocator
		cprintf_colored(TEXT_cyan,"\n3. kfree some of the allocated spaces in BLOCK allocator\n");
		uint32 startOfFreedBlocks[2] = {0};
		uint32 endOfFreedBlocks[2] = {0};
		{
			int idx=0;
			//kfree ALL blocks in first buddy (except two blocks of boot-time)
			for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
			{
				if (l == 5) continue; //skip this level since it has no free blocks due to the previous 16B & 32B alloc's
				idx++;
				kfree(startBlockVAs[idx]);
				numOfAllocBlocksPerSize[l]--;
			}
			idx++ ;

			//kfree ALL allocated 64B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(4) - 1 + PAGE_SIZE / BLOCK_SIZE(5) - 1;
			startOfFreedBlocks[0] = idx;
			for (int s = 0; s < PAGE_SIZE / 64 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[6]--;
			}
			assert(numOfAllocBlocksPerSize[6] == 0);
			endOfFreedBlocks[0] = idx-1;

			//kfree ALL allocated 256B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(7) - 1 ;
			startOfFreedBlocks[1] = idx;
			for (int s = 0; s < PAGE_SIZE / 256 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[8]--;
			}
			assert(numOfAllocBlocksPerSize[8] == 0);
			endOfFreedBlocks[1] = idx-1;
		}

		//4. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc and kfree
		cprintf_colored(TEXT_cyan,"\n4. [BLOCK ALLOCATOR] test kheap_physical_address after kmalloc and kfree [25%]\n");
		correct = 1 ;
		{
			uint32 va, pa;
			for (int i = 1; i <= numOfAllocBlocks; i++)
			{
				va = (uint32)startBlockVAs[i];
				pa = kheap_physical_address(va);
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("4.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				if (isVAInsideFreedAreas(i, startOfFreedBlocks, endOfFreedBlocks, 2))
				{
					expected = 0 ;
				}
				else
				{
					expected = (ptr_table[PTX(va)] & 0xFFFFF000) + (va & 0x00000FFF);
				}
				if (expected != pa)
				{
					if (correct)
					{
						//cprintf_colored(TEXT_TESTERR_CLR,"\nBlock index = %d, level = %d, VA = %x, table entry = %x\nkhep_pa = %x, expected = %x\n",i, levelOfEachAllocBlock[i], va, (ptr_table[PTX(va)]) , pa, expected);
						correct = 0;
						cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kheap_physical_address\n");
					}
				}
			}
		}
		if (correct)	eval+=25 ;
	}

	/*SECOND: PAGE ALLOCATOR*/
	{
		//5. Alloc some spaces in the allocator
		cprintf_colored(TEXT_cyan,"\n5. Alloc some spaces in PAGE allocator\n");
		{
			startOfPageAlloc = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE;
			uint32 res = initial_page_allocations(startOfPageAlloc);
			if (res != 100)
			{
				cprintf_colored(TEXT_TESTERR_CLR,"5. initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
				return eval;
			}
		}
		//6. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc only
		cprintf_colored(TEXT_cyan,"\n6. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc only [20%]\n");
		correct = 1;
		{
			uint32 va;
			uint32 offset = 353;
			uint32 startVA = startOfPageAlloc + offset;
			uint32 endVA = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE + totalRequestedSize;
			int i = 0;
			for (va = startVA; va < endVA; va+=PAGE_SIZE)
			{
				allPAs[i++] = kheap_physical_address(va);
			}
			int ii = i ;
			i = 0;
			int j;
			for (va = startVA; va < endVA; )
			{
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("6.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				for (j = PTX(va); i < ii && j < 1024 && va < endVA; ++j, ++i)
				{
					if (((ptr_table[j] & 0xFFFFF000)+(va & 0x00000FFF))!= allPAs[i])
					{
						if (correct)
						{
							//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
							correct = 0;
							cprintf_colored(TEXT_TESTERR_CLR,"6.2 Wrong kheap_physical_address\n");
						}
					}
					va+=PAGE_SIZE;
				}
			}
		}
		if (correct)	eval+=20 ;

		//7. kfree some of the allocated spaces in PAGE allocator
		cprintf_colored(TEXT_cyan,"\n7. kfree some of the allocated spaces in PAGE allocator\n");
		uint32 startOfFreedAreas[3] = {0};
		uint32 endOfFreedAreas[3] = {0};
		//[PAGE ALLOCATOR]
		{
			uint32 startVA = startOfPageAlloc;

			//kfree 1st 4 MB
			int freeFrames = sys_calculate_free_frames() ;
			int freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[0]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[0], PAGE_SIZE)/PAGE_SIZE ) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.1 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[0] = startVA ;
			endOfFreedAreas[0] = startOfFreedAreas[0] + ROUNDUP(requestedSizes[0], PAGE_SIZE) - 1;

			//kfree 1st 2 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[2]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[2], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[1] = startVA + ROUNDUP(requestedSizes[0], PAGE_SIZE) + ROUNDUP(requestedSizes[1], PAGE_SIZE) ;
			endOfFreedAreas[1] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE) - 1;

			//kfree 1st 1 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[4]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[4], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[2] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE)+ ROUNDUP(requestedSizes[3], PAGE_SIZE) ;
			endOfFreedAreas[2] = startOfFreedAreas[2] + ROUNDUP(requestedSizes[4], PAGE_SIZE) - 1 ;
		}

		//8. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc and kfree
		cprintf_colored(TEXT_cyan,"\n8. [PAGE ALLOCATOR] test kheap_physical_address after kmalloc and kfree [25%]\n");
		correct = 1 ;
		{
			uint32 va;
			uint32 offset = 121;
			uint32 startVA = startOfPageAlloc + offset;
			uint32 endVA = startOfPageAlloc + totalRequestedSize;
			uint32 i = 0;
			for (va = startVA; va < endVA; va+=PAGE_SIZE)
			{
				allPAs[i++] = kheap_physical_address(va);
			}
			int ii = i ;
			i = 0;
			int j;
			for (va = startVA; va < endVA; )
			{
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
					if (correct)
					{ correct = 0; panic("8.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				for (j = PTX(va); i < ii && j < 1024 && va < endVA; ++j, ++i)
				{
					if (isVAInsideFreedAreas(va, startOfFreedAreas, endOfFreedAreas, 3))
					{
						expected = 0 ;
					}
					else
					{
						expected = (ptr_table[j] & 0xFFFFF000) + (va & 0x00000FFF);
					}
					//if (((ptr_table[j] & 0xFFFFF000)+((ptr_table[j] & PERM_PRESENT) == 0? 0 : va & 0x00000FFF)) != allPAs[i])
					if (expected != allPAs[i])
					{
						//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va + j*PAGE_SIZE, (ptr_table[j] & 0xFFFFF000) , allPAs[i]);
						if (correct)
						{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"8.2 Wrong kheap_physical_address\n"); }
					}
					va += PAGE_SIZE;
				}
			}
		}
		if (correct)	eval+=25 ;

		//9. test kheap_physical_address on non-mapped area
		cprintf_colored(TEXT_cyan,"\n9. test kheap_physical_address on non-mapped area [10%]\n");
		correct = 1 ;
		{
			uint32 va;
			uint32 startVA = startOfPageAlloc +totalRequestedSize;
			int ii = (KERNEL_HEAP_MAX - startVA) / PAGE_SIZE;
			uint32 i = 0;
			int j;
			long long va2;
			for (va2 = startVA; va2 < (long long)KERNEL_HEAP_MAX; va2+=PTSIZE)
			{
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, (uint32)va2, &ptr_table);
				if (ptr_table == NULL)
				{
					if (correct)
					{ correct = 0; panic("9.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }
				}
				for (j = 0; i < ii && j < 1024; ++j, ++i)
				{
					//if ((ptr_table[j] & 0xFFFFF000) != allPAs[i])
					unsigned int page_va = startVA+i*PAGE_SIZE;
					unsigned int supposed_kheap_phys_add = kheap_physical_address(page_va);
					//if (((ptr_table[j] & 0xFFFFF000)+((ptr_table[j] & PERM_PRESENT) == 0? 0 : page_va & 0x00000FFF)) != supposed_kheap_phys_add)
					if (supposed_kheap_phys_add != 0)
					{
						//cprintf_colored(TEXT_TESTERR_CLR,"\nVA = %x, table entry = %x, khep_pa = %x\n",va2 + j*PAGE_SIZE, (ptr_table[j]) , allPAs[i]);
						if (correct)
						{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"9.2 Wrong kheap_physical_address\n"); }
					}
				}
			}
		}
		if (correct)	eval+=10 ;
	}
	cprintf_colored(TEXT_light_green,"\ntest kheap_physical_address completed. Eval = %d%\n", eval);

	return 1;

}

int test_kheap_virt_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/

	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	int correct = 1;
	int eval = 0;
	uint32 startOfPageAlloc = 0;
	uint32 expected;

	/*FIRST: BLOCK ALLOCATOR*/
	{
		//1. Alloc some spaces in the allocator
		cprintf_colored(TEXT_cyan,"\n1. Alloc some spaces in BLOCK allocator\n");
		{
			uint32 res = initial_block_allocations();
			startOfPageAlloc = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE;
			if (res != 100)
			{
				cprintf_colored(TEXT_TESTERR_CLR,"initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
				return 0;
			}
		}
		//2. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc only
		cprintf_colored(TEXT_cyan,"\n2. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc only [20%]\n");
		correct = 1 ;
		{
			int i;
			uint32 va, pa;
			for (i = 1; i <= numOfAllocBlocks; i++)
			{
				va = (uint32)startBlockVAs[i];
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("3.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				assert((ptr_table[PTX(va)] & PERM_PRESENT) == PERM_PRESENT);
				pa = (ptr_table[PTX(va)] & 0xFFFFF000)+(va & 0x00000FFF);
				uint32 retrievedVA = kheap_virtual_address(pa);
				uint32 expectedVA = va;
				//cprintf_colored(TEXT_TESTERR_CLR,"va to check = %x\n", va);
				if (retrievedVA != expectedVA)
				{
					if (correct)
					{
						correct = 0;
						cprintf_colored(TEXT_TESTERR_CLR,"3.2 Wrong kheap_virtual_address at block size %d! Expected = %x, Actual = %x\n", i, expectedVA, retrievedVA);
					}
				}
			}
		}
		if (correct)	eval+=20 ;

		//3. kfree some of the allocated spaces in BLOCK allocator
		cprintf_colored(TEXT_cyan,"\n3. kfree some of the allocated spaces in BLOCK allocator\n");
		uint32 startOfFreedBlocks[2] = {0};
		uint32 endOfFreedBlocks[2] = {0};
		{
			int idx=0;
			//kfree ALL blocks in first buddy (except two blocks of boot-time)
			for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
			{
				if (l == 5) continue; //skip this level since it has no free blocks due to the previous 16B & 32B alloc's
				idx++;
				kfree(startBlockVAs[idx]);
				numOfAllocBlocksPerSize[l]--;
			}
			idx++ ;

			//kfree ALL allocated 64B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(4) - 1 + PAGE_SIZE / BLOCK_SIZE(5) - 1;
			startOfFreedBlocks[0] = idx;
			for (int s = 0; s < PAGE_SIZE / 64 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[6]--;
			}
			assert(numOfAllocBlocksPerSize[6] == 0);
			endOfFreedBlocks[0] = idx-1;

			//kfree ALL allocated 256B blocks
			idx += PAGE_SIZE / BLOCK_SIZE(7) - 1 ;
			startOfFreedBlocks[1] = idx;
			for (int s = 0; s < PAGE_SIZE / 256 - 1; ++s)
			{
				kfree(startBlockVAs[idx++]);
				numOfAllocBlocksPerSize[8]--;
			}
			assert(numOfAllocBlocksPerSize[8] == 0);
			endOfFreedBlocks[1] = idx-1;
		}

		//4. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc and kfree
		cprintf_colored(TEXT_cyan,"\n4. [BLOCK ALLOCATOR] test kheap_virtual_address after kmalloc and kfree [25%]\n");
		correct = 1 ;
		{
			uint32 va, pa;
			for (int i = 1; i <= numOfAllocBlocks; i++)
			{
				va = (uint32)startBlockVAs[i];

				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("4.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				pa = (ptr_table[PTX(va)] & 0xFFFFF000) + (va & 0xFFF);

				uint32 retrievedVA = kheap_virtual_address(pa);

				uint32 expectedVA;
				if (isVAInsideFreedAreas(i, startOfFreedBlocks, endOfFreedBlocks, 2))
				{
					expectedVA = 0 ;
				}
				else
				{
					expectedVA = va;
				}
				if (expectedVA != retrievedVA)
				{
					if (correct)
					{
						correct = 0;
						cprintf_colored(TEXT_TESTERR_CLR,"4.2 Wrong kheap_virtual_address at block size %d! Expected = %x, Actual = %x\n", i, expectedVA, retrievedVA);
					}
				}
			}
		}
		if (correct)	eval+=25 ;

	}

	/*SECOND: PAGE ALLOCATOR*/
	{
		//5. Alloc some spaces in the allocator
		cprintf_colored(TEXT_cyan,"\n5. Alloc some spaces in PAGE allocator\n");
		{
			startOfPageAlloc = KERNEL_HEAP_START + numOfAllocPages*PAGE_SIZE;
			uint32 res = initial_page_allocations(startOfPageAlloc);
			if (res != 100)
			{
				cprintf_colored(TEXT_TESTERR_CLR,"5. initial allocations are not correct!\nplease make sure the the kmalloc test is correct before testing the kheap_phys_addr\n");
				return eval;
			}
		}

		//6. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc only
		cprintf_colored(TEXT_cyan,"\n6. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc only [20%]\n");
		int numOfFrames = totalRequestedSize/PAGE_SIZE ;
		correct = 1;
		{
			uint32 va;
			uint32 startVA = startOfPageAlloc ;
			uint32 endVA = startOfPageAlloc + totalRequestedSize;
			int i = 0;
			int j;
			for (va = startVA; va < endVA; )
			{
				uint32 *ptr_table ;
				get_page_table(ptr_page_directory, va, &ptr_table);
				if (ptr_table == NULL)
				{ correct = 0; panic("6.1 one of the kernel tables is wrongly removed! Tables of Kernel Heap should not be removed\n"); }

				for (j = PTX(va); i < numOfFrames && j < 1024 && va < endVA; ++j, ++i)
				{
					uint32 offset = j;
					assert((ptr_table[j] & PERM_PRESENT) == PERM_PRESENT);
					allPAs[i] = (ptr_table[j] & 0xFFFFF000) + offset;
					uint32 retrievedVA = kheap_virtual_address(allPAs[i]);
					//cprintf_colored(TEXT_TESTERR_CLR,"va to check = %x\n", va);
					uint32 expectedVA = (va+offset);
					if (retrievedVA != expectedVA)
					{
						correct = 0;
						cprintf_colored(TEXT_TESTERR_CLR,"6.2 Wrong kheap_virtual_address. Expected = %x, Actual = %x\n", expectedVA, retrievedVA);
					}
					va+=PAGE_SIZE;
				}
			}
		}
		if (correct)	eval+=20 ;

		//7. kfree some of the allocated spaces in PAGE allocator
		cprintf_colored(TEXT_cyan,"\n7. kfree some of the allocated spaces in PAGE allocator\n");
		uint32 startOfFreedAreas[3] = {0};
		uint32 endOfFreedAreas[3] = {0};
		//[PAGE ALLOCATOR]
		{
			uint32 startVA = startOfPageAlloc;

			//kfree 1st 4 MB
			int freeFrames = sys_calculate_free_frames() ;
			int freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[0]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.1 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[0], PAGE_SIZE)/PAGE_SIZE ) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.1 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[0] = startVA ;
			endOfFreedAreas[0] = startOfFreedAreas[0] + ROUNDUP(requestedSizes[0], PAGE_SIZE) - 1;

			//kfree 1st 2 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[2]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[2], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[1] = startVA + ROUNDUP(requestedSizes[0], PAGE_SIZE) + ROUNDUP(requestedSizes[1], PAGE_SIZE) ;
			endOfFreedAreas[1] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE) - 1;

			//kfree 1st 1 MB
			freeFrames = sys_calculate_free_frames() ;
			freeDiskFrames = pf_calculate_free_frames() ;
			kfree(ptr_allocations[4]);
			if ((freeDiskFrames - pf_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n"); }
			if ((sys_calculate_free_frames() - freeFrames) < ROUNDUP(requestedSizes[4], PAGE_SIZE)/PAGE_SIZE) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"7.2 Wrong kfree: pages in memory are not freed correctly\n"); }
			startOfFreedAreas[2] = startOfFreedAreas[1] + ROUNDUP(requestedSizes[2], PAGE_SIZE)+ ROUNDUP(requestedSizes[3], PAGE_SIZE) ;
			endOfFreedAreas[2] = startOfFreedAreas[2] + ROUNDUP(requestedSizes[4], PAGE_SIZE) - 1 ;
		}

		//8. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc and kfree
		cprintf_colored(TEXT_cyan,"\n8. [PAGE ALLOCATOR] test kheap_virtual_address after kmalloc and kfree [25%]\n");
		correct = 1 ;
		{
			uint32 va;
			uint32 startVA = startOfPageAlloc;
			uint32 endVA = startOfPageAlloc + totalRequestedSize;
			uint32 i = 0;

			for (i = 0 ; i < numOfFrames; ++i)
			{
				uint32 retrievedVA = kheap_virtual_address(allPAs[i]);
				uint32 expectedVA ;
				if (isVAInsideFreedAreas(startVA, startOfFreedAreas, endOfFreedAreas, 3))
				{
					expectedVA = 0;
				}
				else
				{
					expectedVA = (startVA + (allPAs[i] & 0xFFF));
				}
				if (retrievedVA != expectedVA)
				{
					if (correct)
					{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"8 Wrong kheap_virtual_address. Expected = %x, Actual = %x\n", expectedVA, retrievedVA); }
				}
				startVA += PAGE_SIZE;
			}
		}
		if (correct)	eval+=25 ;

		//9. test kheap_virtual_address on frames of KERNEL CODE
		cprintf_colored(TEXT_cyan,"\n9. test kheap_virtual_address on frames of KERNEL CODE [10%]\n");
		correct = 1 ;
		{
			uint32 i;
			for (i = 1*Mega; i < (uint32)(end_of_kernel - KERNEL_BASE); i+=PAGE_SIZE)
			{
				uint32 retrievedVA = kheap_virtual_address(i);
				if (retrievedVA != 0)
				{
					if (correct)
					{
						cprintf_colored(TEXT_TESTERR_CLR,"\nPA = %x, retrievedVA = %x\n", i, retrievedVA);
						correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"9 Wrong kheap_virtual_address\n");
					}
				}
			}
		}
		if (correct)	eval+=10 ;
	}
	cprintf_colored(TEXT_light_green,"\ntest kheap_virtual_address completed. Eval = %d%\n", eval);

	return 1;

}


/**********************************************************************************************/
/********************************** KREALLOC TESTING AREA *************************************/
/**********************************************************************************************/
int test_krealloc_FF_page()
{
	panic("not implemented function");
}
int test_krealloc_NF_page()
{
	panic("not implemented function");
}
int test_krealloc_BF_page()
{
	panic("not implemented function");
}
int test_krealloc_WF_page()
{
	panic("not implemented function");
}
int test_krealloc_CF_page()
{
	panic("not implemented function");
}

int test_krealloc_FF_block()
{
	panic("not implemented function");
}
int test_krealloc_NF_block()
{
	panic("not implemented function");
}
int test_krealloc_BF_block()
{
	panic("not implemented function");
}
int test_krealloc_WF_block()
{
	panic("not implemented function");
}
int test_krealloc_CF_block()
{
	panic("not implemented function");
}

int test_krealloc_FF_both()
{
	panic("not implemented function");
}
int test_krealloc_NF_both()
{
	panic("not implemented function");
}
int test_krealloc_BF_both()
{
	panic("not implemented function");
}
int test_krealloc_WF_both()
{
	panic("not implemented function");
}
int test_krealloc_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/*************************** FAST PAGE ALLOCATOR TESTING AREA *********************************/
/**********************************************************************************************/
int test_fast_FF()
{
	panic("not implemented function");
}
int test_fast_NF()
{
	panic("not implemented function");
}
int test_fast_BF()
{
	panic("not implemented function");
}
int test_fast_WF()
{
	panic("not implemented function");
}

bool CheckPossibleHoles(uint32 va, uint32 *possibleHoles, uint32 numOfPossibleHoles)
{
	for (int i = 0; i < numOfPossibleHoles; ++i)
	{
		if (va == possibleHoles[i])
		{
			possibleHoles[i] = 0;
			return 1;
		}
	}
	return 0;
}
int test_fast_CF(uint32 allocSize)
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	panic("not implemented function");
}




/**********************************************************************************************/
/******************************** OLD IMPLEMENTATION AREA *************************************/
/**********************************************************************************************/

int initFreeFrames;
int initFreeDiskFrames ;
uint8 firstCall = 1 ;
int test_three_creation_functions()
{
	panic("not implemented function");
}



extern void kfreeall() ;

int test_kfreeall()
{
	panic("not implemented function");
}


extern void kexpand(uint32 newSize) ;

int test_kexpand()
{
	panic("not implemented function");
}

extern void kshrink(uint32 newSize) ;

int test_kshrink()
{
	panic("not implemented function");
}


int test_kfreelast()
{
	panic("not implemented function");
}

