#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct block_store{
    char* data[BLOCK_STORE_AVAIL_BLOCKS][BLOCK_STORE_AVAIL_BLOCKS];
    bitmap_t* bitmap;
} block_store_t;

//Yuto Wada
//Resources for errno.h: https://www.tutorialspoint.com/cprogramming/c_error_handling.htm
block_store_t *block_store_create()
{
    int errornum;

    //Allocate memory for the block that is being created
    block_store_t *block = malloc(sizeof(block_store_t));
    if(block == NULL){
        //errno number stuff
        errornum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error Null Check: %s\n", strerror( errornum ));
        return NULL;
    }
    //Create a bitmap with 256 - 1 as the available space
    block->bitmap = bitmap_create(BLOCK_STORE_AVAIL_BLOCKS);
    return block;
}

//Yuto Wada
void block_store_destroy(block_store_t *const bs)
{
    int errornum;

    if (bs == NULL){
        errornum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error Null Check: %s\n", strerror( errornum )); 
        return;
    }

    //If the parameter is not null, destroy the bitmap that is allocated and free the memory
    bitmap_destroy(bs->bitmap);
    free(bs);
    return; 
}

//Yuto Wada
size_t block_store_allocate(block_store_t *const bs)
{
    //If bs is NULL, return SIZE_MAX (Stated in the test cases)
    if (bs == NULL){
        return SIZE_MAX;
    }

    //Find where the first zero occurs
    size_t adressZero = bitmap_ffz(bs->bitmap);


    if (adressZero == SIZE_MAX || adressZero == BLOCK_STORE_AVAIL_BLOCKS) {
        return SIZE_MAX;
    }

    //Set the bs to where the first zero is.
    bitmap_set(bs->bitmap, adressZero);
    return adressZero;
}

//Yuto Wada
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if(bs == NULL || (block_id > BLOCK_STORE_AVAIL_BLOCKS)) {
        return 0;
    }

    //If the bit is already set, exit
    if(bitmap_test(bs->bitmap, block_id) == 1) {
        return 0; 
    }

    //Set the bit to be the requested block
    bitmap_set(bs->bitmap, block_id);

    //If the bit is not used or set, something went wrong
    if(bitmap_test(bs->bitmap, block_id) == 0) {
        return 0; 
    }

    return 1;
}

//Yuto Wada
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    //checks if the block store is null
    if(bs != NULL){
        //resets the bit representing the selected block
        bitmap_reset(bs->bitmap, block_id);
    }
    return;
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    //checks if the block store is null
    if(bs != NULL) {
        //returns the total number of set bits the the block store's bitmap
        return bitmap_total_set(bs->bitmap);
    }
    //returns zero if the block store is null
    return SIZE_MAX;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    //checks if the block store is null
    if(bs != NULL) {
        //returns the number of unset bits in the block store's bitmap by subtracting the set bits from the total bits
        return BLOCK_STORE_AVAIL_BLOCKS - bitmap_total_set(bs->bitmap);
    }
    //returns zero if the block store is null
    return SIZE_MAX; 
}

//Yuto Wada
size_t block_store_get_total_blocks()
{
    //returns the constant that represents the total blocks in the block store
    return BLOCK_STORE_AVAIL_BLOCKS;
}

//Micah 
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    // error checking
    if(bs == NULL) return 0;
    if(block_id > BLOCK_STORE_NUM_BLOCKS) return 0; // double check this
    if(buffer == NULL) return 0;
    
    // read data from block_store into buffer
    size_t i = 0;
    while(i < BLOCK_STORE_AVAIL_BLOCKS)
    {
        ((char*)buffer)[i] = (*(bs->data))[block_id][i];
    }

    return i;
}

//Micah
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{    
    // error checking
    if(bs == NULL) return 0;
    if(block_id> BLOCK_STORE_NUM_BLOCKS) return 0;
    if(buffer == NULL) return 0;
    
    // read data from buffer into block_store
    size_t i = 0;
    while(i < BLOCK_STORE_AVAIL_BLOCKS)
    {
        (*(bs->data))[block_id][i] = ((char*)buffer)[i];
    }

    return i;
}

//Micah
block_store_t *block_store_deserialize(const char *const filename)
{
    UNUSED(filename);
    return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    UNUSED(bs);
    UNUSED(filename);
    return 0;
}
