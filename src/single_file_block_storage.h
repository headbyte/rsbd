//
// Created by firstbyte on 02/01/2020.
//

#ifndef RSBD_SINGLE_FILE_BLOCK_STORAGE_H
#define RSBD_SINGLE_FILE_BLOCK_STORAGE_H

#include "block_storage.h"

namespace rsbd {

    struct single_file_block_storage_index : public serializable {

        size_t block_map_start = 0;
        std::vector<char> block_map;

        size_t hashes_start = 0;
        std::vector<block_hash> hashes;

        block_id_t starting_block = 0; // TODO: unused at the moment, UPDATE GET,SET FUNCTIONS TO UTILISE THIS
        block_size_t block_size = 0;
        block_id_t block_count = 0;
        char magic[4]; // "RSBD"

        //
        void init(block_id_t block_count) {
            std::memcpy(magic, "RSBD", 4);
            set_block_count(block_count);
        }

        block_id_t get_block_count() const {
            return block_count;
        }


        size_t get_hashes_start() {
            if (hashes_start != 0)
                return hashes_start;

            hashes_start = get_block_map_start() + block_count;
            return hashes_start;
        }

        size_t get_block_map_start() {
            if (block_map_start != 0)
                return block_map_start;

            block_map_start = block_size * block_count;
            return block_map_start;
        }

        void update_block_information(std::ostream &output, block &b, bool delete_block = false) {
            output.seekp(get_block_map_start() + b.id, std::ios_base::beg);
            char exists = 1;
            if (delete_block) {
                exists = 0;
                output.write(&exists, 1);
                return;
            }
            output.write(&exists, 1);
            block_map[b.id] = 1;


            auto hash = b.get_hash();

            output.seekp(get_hashes_start() + (b.id * sizeof(hash.hash)), std::ios_base::beg);
            output.write((char *) hash.hash, sizeof(hash.hash));
            std::memcpy(hashes[b.id].hash, hash.hash, sizeof(hash.hash));
            hashes[b.id].calculated = true;
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
            binary_output_stream bs(output);

            bs.write(block_map.data(), block_count);
            for (int i = 0; i < block_count; ++i) {
                bs.write(hashes[i].hash, sizeof(hashes[i].hash));
            }

            bs.write(starting_block);
            bs.write(block_size);
            bs.write(block_count);
            bs.write(magic, 4);
        }

        void reverse_deserialize(std::istream &input) {
            binary_input_stream bis(input);

            bis.reverse_read(magic, 4);
            if (std::memcmp(magic, "RSBD", 4) != 0) {
                throw std::logic_error("header magic doesn't match");
            }

            bis.reverse_read(block_count);
            bis.reverse_read(block_size);
            bis.reverse_read(starting_block);

            set_block_count(block_count);

            int i = block_count;
            while (i-- > 0) {
                bis.reverse_read(hashes[i].hash, sizeof(hashes[i].hash));
            }
            bis.reverse_read(this->block_map.data(), block_count);
        }

        void deserialize(std::istream &input) override {
            throw std::logic_error("you shouldn't deserialize this in normal order");
        }
    };

    struct single_file_block_storage : public block_storage {
    protected:
        void open_stream() {
            stream_ptr->seekg(0, std::ios_base::end);
            header.reverse_deserialize(*stream_ptr);
        }

        void create_stream(block_size_t block_size, block_id_t block_count) {
            block b;
            b.init_empty(block_size);

            char empty_byte = 0;
            for (int i = 0; i < block_count; ++i) {
                stream_ptr->write(&empty_byte, 1);
            }

            header.init(block_count);
            header.block_size = block_size;

            header.serialize(*stream_ptr);
        }

    public:

        void open(std::shared_ptr<std::iostream> stream) {
            stream_ptr = stream;
            open_stream();
        }

        void open(std::string path) {
            this->path = path;

            stream_ptr = std::make_shared<std::fstream>(
                    path,
                    std::ios_base::in | std::ios_base::out | std::ios_base::binary
            );

            if (!is_ready()) {
                throw std::runtime_error("file open failed");
            }

            open_stream();
        }

        const std::string &get_path() const {
            return path;
        }

        void create(std::shared_ptr<std::iostream> stream, block_size_t block_size, block_id_t block_count) {
            stream_ptr = stream;
            create_stream(block_size, block_count);
        }


        void create(std::string path, block_size_t block_size, block_id_t block_count) {
            this->path = path;
            stream_ptr = std::make_shared<std::fstream>(
                    path,
                    std::ios_base::trunc | std::ios_base::in | std::ios_base::out | std::ios_base::binary
            );

            if (!is_ready()) {
                throw std::runtime_error("file create failed");
            }

            create_stream(block_size, block_count);
        }

        bool is_ready() override {
            return stream_ptr != nullptr && !stream_ptr->bad();
        }

        size_t get_block_position(block_id_t id) {
            // TODO: CHECK VALUES HERE with assert
            return header.block_size * id;
        }

        virtual bool get_block(block_id_t id, block &b) override {
            if (header.block_map[id] == 0) {
                return false;
            }

            if (!is_ready()) {
                throw std::logic_error("not ready");
            }

            b.id = id;
            b.size = get_block_size(id);
            b.data.resize(b.size);

            std::lock_guard<std::mutex> guard(stream_mutex);
            stream_ptr->seekg(get_block_position(id), std::ios_base::beg);
            stream_ptr->read((char *) b.data.data(), b.size);
            return true;
        }

        virtual bool has_block(rsbd::block_id_t id) override {
            return header.block_map[id] == 1;
        }

        virtual block_size_t get_block_size(rsbd::block_id_t id) override {
            return header.block_size;
        }

        bool delete_block(block_id_t id) override {
            return false;
        }

        virtual void set_block(rsbd::block &b) override {
            if (b.id < 0 && b.id >= header.block_count) {
                throw std::range_error("out of range");
            }

            if (!is_ready()) {
                throw std::logic_error("not ready");
            }

            std::lock_guard<std::mutex> guard(stream_mutex);
            stream_ptr->seekp(get_block_position(b.id), std::ios_base::beg);
            stream_ptr->write((char *) b.data.data(), b.size);

            header.update_block_information(*stream_ptr, b);
        }

        virtual block_id_t get_block_count() override {
            return header.block_count;
        }

        void close() {
            std::lock_guard<std::mutex> guard(stream_mutex);
            stream_ptr = nullptr;
        }

        const block_hash &get_block_hash_from_header(block_id_t id) {
            return header.hashes[id];
        }

    protected:
        single_file_block_storage_index header;
        std::string path;

        std::shared_ptr<std::iostream> stream_ptr{nullptr};
        //std::unique_ptr<std::iostream> stream_ptr;
        //std::fstream stream;

        std::mutex stream_mutex;
    };
}

#endif //RSBD_SINGLE_FILE_BLOCK_STORAGE_H
