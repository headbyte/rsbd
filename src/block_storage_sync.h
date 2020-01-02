//
// Created by firstbyte on 02/01/2020.
//

#ifndef RSBD_BLOCK_STORAGE_SYNC_H
#define RSBD_BLOCK_STORAGE_SYNC_H

#include "block_storage.h"

namespace rsbd {
    struct block_storage_sync {
        block_storage_sync(block_storage &from, block_storage &to) : from(from), to(to) {}

        void synchronise() {
            auto block_count = from.get_block_count();
            if (block_count != to.get_block_count()) {
                throw std::logic_error("block counts of from and to storages are not equal");
            }


            auto block_size = from.get_block_size(0);
            if (block_size != to.get_block_size(0)) {
                throw std::logic_error("block size of from and to storages are not equal");
            }

            for (int i = 0; i < block_count; ++i) {
                auto from_hash = from.get_block_hash(i);
                auto to_hash = to.get_block_hash(i);
                if (!(from_hash == to_hash)) {
                    block from_block;
                    if (!from.get_block(i, from_block)) {
                        throw std::logic_error("reading block failed from source block disk");
                    }
                    to.set_block(from_block);
                }
            }
        }

    protected:
        block_storage &from;
        block_storage &to;
    };
}

#endif //RSBD_BLOCK_STORAGE_SYNC_H
