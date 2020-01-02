//
// Created by firstbyte on 02/01/2020.
//

#ifndef RSBD_BLOCK_STORAGE_H
#define RSBD_BLOCK_STORAGE_H

#include "block.h"

namespace rsbd {

    struct block_storage {
        uint8_t uuid[16];

        virtual uint32_t get_block_size(uint64_t id) = 0;

        virtual bool get_block(uint64_t id, block &b) = 0;

        virtual void set_block(block &b) = 0;

        virtual bool has_block(uint64_t id) = 0;

        virtual bool delete_block(uint64_t id) = 0;

        virtual uint64_t get_block_count() = 0;

        virtual bool is_ready() = 0;
    };
}

#endif //RSBD_BLOCK_STORAGE_H
