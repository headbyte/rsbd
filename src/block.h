/*
 * block.h
 *
 *  Created on: 1 Jan 2020
 *      Author: firstbyte
 */


#ifndef SRC_BLOCK_H_
#define SRC_BLOCK_H_

#include <cstdint>
#include <vector>
#include <cryptopp/sha.h>
#include <fstream>
#include <mutex>
#include <iostream>
#include "binary.h"

namespace rsbd {

    typedef uint64_t block_id_t;
    typedef uint32_t block_size_t;
    typedef std::vector<uint8_t> buffer;

    struct block;

    struct block_hash {
        bool calculated = false;
        uint8_t hash[CryptoPP::SHA256::DIGESTSIZE];

        bool operator==(const block_hash &other) const {
            return std::memcmp(hash, other.hash, sizeof(hash)) == 0;
        }
    };

    struct block {

        block() {}

        block(block_id_t id, block_size_t size) : id(id), size(size) {
            data.resize(size, 0);
        }

        block_id_t id;
        block_size_t size;
        buffer data;

        block_hash get_hash() {
            if (this->hash.calculated) {
                return this->hash;
            }

            CryptoPP::SHA256 hash;
            hash.CalculateDigest(this->hash.hash, data.data(), size);
            this->hash.calculated = true;
            return this->hash;
        }

    protected:
        block_hash hash;
    };


    struct multi_file_block_storage {

    };

}


#endif /* SRC_BLOCK_H_ */
