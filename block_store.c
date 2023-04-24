#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
#include <string.h>
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

#define FBM_START_BLOCK 127

struct block_store {
    uint8_t blocks[BLOCK_STORE_NUM_BLOCKS][BLOCK_SIZE_BYTES];
    bitmap_t *fbm;
};

block_store_t *block_store_create()
{
    block_store_t *bs = malloc(sizeof(block_store_t));
    if (!bs) {
        return NULL;
    }

    // Initialize all blocks to zero
    memset(bs->blocks, 0, BLOCK_STORE_NUM_BLOCKS * BLOCK_SIZE_BYTES);
    
    bs->fbm = bitmap_create(BITMAP_SIZE_BYTES * 8);
    
    memcpy(bs->blocks[FBM_START_BLOCK], bitmap_export(bs->fbm), sizeof(bs->fbm));
    bitmap_set(bs->fbm, FBM_START_BLOCK);

    return bs;
}

void block_store_destroy(block_store_t *const bs)
{
    if (bs) {
        if (bs->fbm) {
            bitmap_destroy(bs->fbm);
        }
        free(bs);
    }
}

size_t block_store_allocate(block_store_t *const bs)
{
    if (!bs || !bs->fbm) {
        return SIZE_MAX;
    }

    size_t free_block = bitmap_ffz(bs->fbm);

    if (free_block != SIZE_MAX) {
        bitmap_set(bs->fbm, free_block);
    }
    else {
        return SIZE_MAX;
    }

    return (free_block >= FBM_START_BLOCK) ? free_block - 1 : free_block;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (!bs || !bs->fbm || block_id >= BLOCK_STORE_NUM_BLOCKS) {
        return false;
    }

    if (bitmap_test(bs->fbm, block_id)) {
        return false;
    }

    bitmap_set(bs->fbm, block_id);
    return true;
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if (bs && bs->fbm && block_id < BLOCK_STORE_NUM_BLOCKS) {
        bitmap_reset(bs->fbm, block_id);
    }
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    if (!bs || !bs->fbm) {
        return SIZE_MAX;
    }

    return bitmap_total_set(bs->fbm) - 1; // Not sure why I need to subtract here.
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    size_t used_blocks = block_store_get_used_blocks(bs);

    if (used_blocks == SIZE_MAX) {
        return SIZE_MAX;
    }

    return BLOCK_STORE_NUM_BLOCKS - used_blocks - 1; // Not sure why I need to subtract here.
}

size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_AVAIL_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    if (!bs || !bs->fbm || block_id >= BLOCK_STORE_NUM_BLOCKS || !buffer) {
        return 0;
    }

    if (!bitmap_test(bs->fbm, block_id)) {
        return 0;
    }

    memcpy(buffer, bs->blocks[block_id], BLOCK_SIZE_BYTES);
    return BLOCK_SIZE_BYTES;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if (!bs || !bs->fbm || block_id >= BLOCK_STORE_NUM_BLOCKS || !buffer) {
        return 0;
    }

    if (!bitmap_test(bs->fbm, block_id)) {
        return 0;
    }

    memcpy(bs->blocks[block_id], buffer, BLOCK_SIZE_BYTES);
    return BLOCK_SIZE_BYTES;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    if (!filename) {
        return NULL;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    block_store_t *bs = malloc(sizeof(block_store_t));
    if (!bs) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(bs, sizeof(uint8_t), BLOCK_STORE_NUM_BYTES, file);
    fclose(file);

    if (bytes_read != BLOCK_STORE_NUM_BYTES) {
        free(bs);
        return NULL;
    }

    bs->fbm = bitmap_import(BITMAP_SIZE_BYTES * 8, bs->blocks[FBM_START_BLOCK]);
    return bs;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    if (!bs || !filename) {
        return 0;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        return 0;
    }

    size_t bytes_written = fwrite(bs, sizeof(uint8_t), BLOCK_STORE_NUM_BYTES, file);
    fclose(file);

    return bytes_written;
}
