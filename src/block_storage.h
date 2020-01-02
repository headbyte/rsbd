//
// Created by firstbyte on 02/01/2020.
//

#ifndef RSBD_BLOCK_STORAGE_H
#define RSBD_BLOCK_STORAGE_H

#include "block.h"

namespace rsbd {

    struct block_storage {
        uint8_t uuid[16];

        virtual block_size_t get_block_size(block_id_t id) = 0;

        virtual bool get_block(block_id_t id, block &b) = 0;

        virtual void set_block(block &b) = 0;

        virtual bool has_block(block_id_t id) = 0;

        virtual bool delete_block(block_id_t id) = 0;

        virtual block_hash get_block_hash(block_id_t id) = 0;

        virtual block_id_t get_block_count() = 0;

        virtual bool is_ready() = 0;
    };
}

#endif //RSBD_BLOCK_STORAGE_H
