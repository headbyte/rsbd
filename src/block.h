/*
 * block.h
 *
 *  Created on: 1 Jan 2020
 *      Author: firstbyte
 */

#include <cstdint>
#include <vector>
#include <cryptopp/sha.h>
#include <fstream>
#include <mutex>

#ifndef SRC_BLOCK_H_
#define SRC_BLOCK_H_

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

    struct block_storage {
        uint8_t uuid[16];

        virtual block_size_t get_block_size(block_id_t id) = 0;

        virtual bool get_block(block_id_t id, block &b) = 0;

        virtual void set_block(block &b) = 0;

        virtual bool has_block(block_id_t id) = 0;

        virtual block_id_t get_block_count() = 0;

        virtual bool is_ready() = 0;
    };

    struct serializable {
        virtual void serialize(std::ostream &output) = 0;

        virtual void deserialize(std::istream &input) = 0;
    };

    template<class T>
    void write(std::ostream &output, T &data) {
        output.write((char *) &data, sizeof(data));
    }

    template<class T>
    void write(std::ostream &output, const T *data, size_t length) {
        output.write((char *) data, sizeof(T) * length);
    }


    template<class T>
    void read(std::istream &input, T &data) {
        T temp;
        input.read((char *) &temp, sizeof(data));
        data = temp;
    }

    template<class T>
    void read(std::istream &input, T *data, int length) {
        input.read((char *) data, sizeof(T) * length);
    }

    template<class T>
    void reverse_read(std::istream &input, T &data) {
        size_t byte_count = sizeof(T);
        input.seekg(-byte_count, std::ios_base::cur);
        T temp;
        input.read((char *) &temp, byte_count);
        data = temp;
        input.seekg(-byte_count, std::ios_base::cur);
    }

    template<class T>
    void reverse_read(std::istream &input, T *data, int length) {
        size_t byte_count = sizeof(T) * length;
        input.seekg(-byte_count, std::ios_base::cur);
        input.read((char *) data, byte_count);
        input.seekg(-byte_count, std::ios_base::cur);
    }

    struct single_file_block_storage_index : public serializable {
        std::vector<char> block_map;
        std::vector<block_hash> hashes;

        block_id_t starting_block = 0;
        block_size_t block_size = 0;
        block_id_t block_count = 0;
        char magic[4]; // "RSBD"

        //
        void init(block_id_t block_count) {
            std::memcpy(magic, "RSBD", 4);

        }

        block_id_t get_block_count() const {
            return block_count;
        }

        void set_block_count(block_id_t block_count) {
            single_file_block_storage_index::block_count = block_count;
            block_count = block_count;
            block_map.resize(block_count);
            hashes.resize(block_count);
            for (size_t i = 0; i < block_count; i++) {
                block_map[i] = false;
                hashes[i].calculated = false;
                std::memset(hashes[i].hash, 0, sizeof(hashes[i].hash));
            }
        }

        void serialize(std::ostream &output) override {
            write(output, block_map.data(), block_count);
            for (int i = 0; i < block_count; ++i) {
                write(output, hashes[i].hash, sizeof(hashes[i].hash));
            }

            write(output, starting_block);
            write(output, block_size);
            write(output, block_count);
            write(output, magic, 4);
        }

        void reverse_deserialize(std::istream &input) {
            reverse_read(input, magic, 4);
            reverse_read(input, block_count);
            reverse_read(input, block_size);
            reverse_read(input, starting_block);

            set_block_count(block_count);

            for (int i = 0; i < block_count; ++i) {
                reverse_read(input, hashes[i].hash, sizeof(hashes[i].hash));
            }
            reverse_read(input, this->block_map.data(), block_count);
        }

        void deserialize(std::istream &input) override {
            throw std::logic_error("you shouldn't deserialize this in normal order");
//            read(input, this->block_map)
//
//            read(input, starting_block);
//            read(input, block_size);
//            read(input, block_count);
//            read(input, magic, 4);
        }
    };

    struct single_file_block_storage : public block_storage {
        void open(std::string path) {
            this->path = path;
            stream.open(path);

            if (stream.good()) {
                throw std::runtime_error("file open failed");
            }

            stream.seekg(0, std::ios_base::end);
            header.deserialize(stream);
        }

        const std::string &get_path() const {
            return path;
        }

        void create(std::string path, block_size_t block_size, block_id_t block_count) {
            this->path = path;
            stream.open(path);
            if (stream.good()) {
                throw std::runtime_error("file open failed");
            }

            size_t total_file_size = block_count * block_size;
            stream.seekp(total_file_size - 1);
            char empty_byte = 0;
            stream.write(&empty_byte, 1);

            header.init(block_count);
            header.block_size = block_size;

            header.serialize(stream);
        }

        bool is_ready() override {
            return stream.is_open() && stream.good();
        }

        size_t get_block_position(block_id_t id) {
            // TODO: CHECK VALUES HERE
            return header.block_size * id;
        }

        virtual bool get_block(block_id_t id, block &b) override {
            if (header.block_map[id] == 0) {
                return false;
            }
            b.id = id;
            b.size = get_block_size(id);
            b.data.resize(b.size);

            std::lock_guard<std::mutex> guard(stream_mutex);
            stream.seekg(get_block_position(id), std::ios_base::beg);
            stream.read((char *) b.data.data(), b.size);
            return true;
        }

        virtual bool has_block(rsbd::block_id_t id) override {
            return header.block_map[id] == 1;
        }

        virtual block_size_t get_block_size(rsbd::block_id_t id) override {
            return header.block_size;
        }

        virtual void set_block(rsbd::block &b) override {

        }

        virtual block_id_t get_block_count() override {
            return header.block_count;
        }

    protected:
        single_file_block_storage_index header;
        std::string path;
        std::fstream stream;
        std::mutex stream_mutex;
    };

    struct multi_file_block_storage {

    };

}


#endif /* SRC_BLOCK_H_ */
