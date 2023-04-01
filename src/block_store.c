#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
#include <unistd.h>
#include <fcntl.h>
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct block_store{
    char* data[BLOCK_STORE_AVAIL_BLOCKS][BLOCK_STORE_AVAIL_BLOCKS];
    bitmap_t* bitmap;
} block_store_t;

block_store_t *block_store_create()
{
    block_store_t *block = malloc(sizeof(block_store_t*));
    if(block == NULL){
        return NULL;
    }
    block->bitmap = bitmap_create(BLOCK_STORE_AVAIL_BLOCKS);
    return block;
}

void block_store_destroy(block_store_t *const bs)
{
    bitmap_destroy(bs->bitmap);
    free(bs);
    return;
}
size_t block_store_allocate(block_store_t *const bs)
{
    if (bs == NULL){
        return BLOCK_STORE_AVAIL_BLOCKS;
    }

    size_t adressZero = bitmap_ffz(bs->bitmap);

    if (adressZero == SIZE_MAX || adressZero == BLOCK_STORE_AVAIL_BLOCKS) return BLOCK_SIZE_BYTES;

    bitmap_set(bs->bitmap, adressZero); //set first zero
    return adressZero;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if(bs == NULL || block_id > BLOCK_STORE_AVAIL_BLOCKS) return 0;

    if(bitmap_test(bs->bitmap, block_id) == 1) return 0; //if the bit is set, terminate

    bitmap_set(bs->bitmap, block_id);

    if(bitmap_test(bs->bitmap, block_id) == 0) return 0; //if the bit is not set, terminate

    return 1;
}

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
    return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    //checks if the block store is null
    if(bs != NULL) {
        //returns the number of unset bits in the block store's bitmap by subtracting the set bits from the total bits
        return BLOCK_STORE_AVAIL_BLOCKS - bitmap_total_set(bs->bitmap);
    }
    //returns zero if the block store is null
    return 0; 
}

size_t block_store_get_total_blocks()
{
    //returns the constant that represents the total blocks in the block store
    return BLOCK_STORE_AVAIL_BLOCKS;
}

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

block_store_t *block_store_deserialize(const char *const filename)
{
    //checks if the filename is null
    if(filename == NULL) return NULL;

    //open the file 
    int fd = open(filename, O_RDONLY);
    //check if there was an error while opening the file
    if(fd == -1) {
        return NULL;
    }

    //creates the block store
    block_store_t* bs = block_store_create();
    //checks if the block store was successfully created
    if(bs == NULL) return NULL;

    //initialze buffer
    void* buf[BLOCK_STORE_NUM_BLOCKS];
    
    //reads through the array of ints in the file
    for(size_t i = 0; i < BLOCK_STORE_NUM_BLOCKS; i++) {
        //read the current block and reads it into the buffer
        size_t bytes_read = read(fd, buf, BLOCK_STORE_NUM_BLOCKS); 
        //check if the read was successful
        if(bytes_read == 0) {
            return NULL;
        }
        //writes the buffer for the block into the block store
        size_t num_bytes = block_store_write(bs, i, buf);
        //checks if the block store is correctly written to
        if(num_bytes == 0) {
            return NULL;
        }
    } 

    return bs;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    //checks if the block store is null
    if(bs == NULL) return 0;
    //checks if the filename is null
    if(filename == NULL) return 0;

    //open the file 
    int fd = open(filename, O_WRONLY);
    //check if there was an error while opening the file
    if(fd == -1) {
        return 0;
    }

    //set the buffer
    void* buf[BLOCK_STORE_NUM_BLOCKS];
    size_t total_bytes = 0; //initialize total bytes written to zero

    //writes each block to the file
    for(size_t i = 0; i < BLOCK_STORE_NUM_BLOCKS; i++) {
        //read block into buffer
        size_t num_bytes = block_store_read(bs, i, buf);
        //check if buffer was correctly written to
        if(num_bytes == 0) {
            return 0;
        }
        //write buffer of block to file
        size_t bytes_written = write(fd, buf, BLOCK_STORE_NUM_BLOCKS);
        //check if the file was correctly written to
        if(bytes_written == 0) {
            return 0;
        }
        //add the bytes written for the block to the total bytes written
        total_bytes += bytes_written;
    }

    //return the total number of bytes written to files
    return total_bytes;
}
