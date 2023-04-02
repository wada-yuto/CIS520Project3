#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct block_store{
    char* data[BLOCK_STORE_AVAIL_BLOCKS][BLOCK_STORE_AVAIL_BLOCKS];
    bitmap_t* bitmap;
} block_store_t;

//Yuto Wada
block_store_t *block_store_create()
{
    block_store_t *block = malloc(sizeof(block_store_t));
    if(block == NULL){
        return NULL;
    }
    block->bitmap = bitmap_create(BLOCK_STORE_AVAIL_BLOCKS);
    return block;
}

//Yuto Wada
void block_store_destroy(block_store_t *const bs)
{
    if (bs != NULL){
        bitmap_destroy(bs->bitmap);
        free(bs);
        return;  
    }
}

//Yuto Wada
size_t block_store_allocate(block_store_t *const bs)
{
    if (bs == NULL){
        return SIZE_MAX;
    }

    size_t adressZero = bitmap_ffz(bs->bitmap);

    if (adressZero == SIZE_MAX || adressZero == BLOCK_STORE_AVAIL_BLOCKS) {
        return SIZE_MAX;
    }

    bitmap_set(bs->bitmap, adressZero); //set first zero
    return adressZero;
}

//Yuto Wada
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if(bs == NULL || block_id > BLOCK_STORE_AVAIL_BLOCKS) return 0;

    if(bitmap_test(bs->bitmap, block_id) == 1) return 0; //if the bit is set, terminate

    bitmap_set(bs->bitmap, block_id);

    if(bitmap_test(bs->bitmap, block_id) == 0) return 0; //if the bit is not set, terminate

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
    while ( i < BLOCK_SIZE_BYTES )
    {
        strncpy( buffer+i, (bs->data)[block_id][i], (size_t)1 );
        //printf("i: %ld\n", i);
	i++;
    }

    return i;

    // memcpy(buffer, bs->data[block_id],BLOCK_STORE_AVAIL_BLOCKS); //copy block id into buffer
    // return BLOCK_STORE_AVAIL_BLOCKS;
}

//Micah
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{    
    // error checking
    if(bs == NULL) return 0;
    if(block_id > BLOCK_STORE_NUM_BLOCKS) return 0;
    if(buffer == NULL) return 0;
  
    // read data from block_store into buffer 
    size_t i = 0;
    while ( i < BLOCK_SIZE_BYTES )
    {
        (bs->data)[block_id][i] = calloc(1, sizeof(char));
	strncpy( (bs->data)[block_id][i], buffer+i, (size_t)1 );	
	i++;
    }

    return i;
}

//Katia
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

    //import bitmap from file
    // void* bitmap_buf = malloc(sizeof(uint8_t));
    // read(fd, bitmap_buf, sizeof(uint8_t));
    // bitmap_t* imported_bitmap = bitmap_import(sizeof(uint8_t), bitmap_buf);
    // bs->bitmap = imported_bitmap;

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
        //printf("%p\n", buf);
        if(is_zeroed(buf)) {
            printf("is being used\n");
            //writes the buffer for the block into the block store
            size_t num_bytes = block_store_write(bs, i, buf);
            //checks if the block store is correctly written to
            if(num_bytes == 0) {
                return NULL;
            }
            //mark block as used
            bool requested = block_store_request(bs, i);
            if(!requested) {
                return NULL;
            }
        }
    } 

    //close the file
    close(fd);

    return bs;
}

bool is_zeroed(void* buf) {
    //checks if the buffer is filled with zeroes
    for(size_t j = 0; j < BLOCK_STORE_NUM_BLOCKS; j++) {
            if(buf+j != 0) return false;
    }
    return true;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    //checks if the block store is null
    if(bs == NULL) return 0;
    //checks if the filename is null
    if(filename == NULL) return 0;

    //open the file 
    int fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
    //check if there was an error while opening the file
    if(fd == -1) {
        perror("Error: ");
        printf("failed to open file\n");
        return 0;
    }

    //set the buffer
    void* buf = malloc(BLOCK_STORE_NUM_BLOCKS);
    size_t total_bytes = 0; //initialize total bytes written to zero

    //write bitmap to file
    // void* bitmap_buf = malloc(sizeof(uint8_t));
    // bitmap_export(bs->bitmap);
    // write(fd, bitmap_buf, sizeof(uint8_t));

    //writes each block to the file
    for(size_t i = 0; i < BLOCK_STORE_NUM_BLOCKS; i++) {
        //write the contents of the used blocks
        if(bitmap_test(bs->bitmap, i)) {
            //printf("%ld is set\n", i);
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
        //write a placeholder for blocks that are unused
        else {
            //printf("%ld is placeholder\n", i);
            void* placeholder_buffer = malloc(BLOCK_STORE_NUM_BLOCKS);
            memset(placeholder_buffer, 0, BLOCK_STORE_NUM_BLOCKS);
            write(fd, placeholder_buffer, BLOCK_STORE_NUM_BLOCKS);
            total_bytes += 256;
        }
    }

    //close the file
    close(fd);

    //return the total number of bytes written to files
    return total_bytes;
}
