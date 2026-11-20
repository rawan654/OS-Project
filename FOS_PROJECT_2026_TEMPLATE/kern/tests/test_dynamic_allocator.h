/*
 * test_dynamic_allocator.h
 *
 *  Created on: Oct 14, 2022
 *      Author: HP
 */

#ifndef KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_
#define KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif
#include <inc/dynamic_allocator.h>

/*2025*/
int  test_initialize_dynamic_allocator();
void test_alloc_block();
void test_free_block();
void test_realloc_block();
int check_blocks_in_page(void* pageVA, struct BlockMetaData* expectedBlocks, uint32 numOfBlocks);
int check_free_block_lists(uint32 expectedSizes[BUDDY_UPPER_LEVEL+1], uint32* pagesVAs, uint32 numOfPages);


#endif /* KERN_TESTS_TEST_DYNAMIC_ALLOCATOR_H_ */
