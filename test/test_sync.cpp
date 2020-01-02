//
// Created by firstbyte on 02/01/2020.
//

#include <catch2/catch.hpp>
#include <sstream>
#include <iostream>
#include "block_storage_sync.h"
#include "single_file_block_storage.h"
#include <random>

using namespace rsbd;

template<class E, class T>
inline void
hex_dump(const void *data, std::size_t length, std::basic_ostream<E, T> &stream = std::cout, std::size_t columns = 16) {
    const char *const start = static_cast<const char *>(data);
    const char *const end = start + length;
    const char *line = start;
    while (line != end) {
        stream.width(4);
        stream.fill('0');
        stream << std::hex << line - start << " : ";
        std::size_t lineLength = std::min(columns, static_cast<std::size_t>(end - line));
        for (std::size_t pass = 1; pass <= 2; ++pass) {
            for (const char *next = line; next != end && next != line + columns; ++next) {
                char ch = *next;
                switch (pass) {
                    case 1:
                        stream << ((ch < 32 || ch > 128) ? '.' : ch);
                        break;
                    case 2:
                        if (next != line)
                            stream << " ";
                        stream.width(2);
                        stream.fill('0');
                        stream << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch));
                        break;
                }
            }
            if (pass == 1 && lineLength != columns)
                stream << std::string(columns - lineLength, ' ');
            stream << " ";
        }
        stream << std::endl;
        line = line + lineLength;
    }
}

TEST_CASE("single file block storage sync test", "[sync]") {
    auto from_memory = std::make_shared<std::stringstream>();
    auto to_memory = std::make_shared<std::stringstream>();

    single_file_block_storage from_storage;
    REQUIRE_NOTHROW(from_storage.create(from_memory, 4, 16));
    for (int i = 0; i < 16; ++i) {
        block b(i, 4);
        *(uint32_t *) b.data.data() = i;
        from_storage.set_block(b);
    }

    single_file_block_storage to_storage;
    REQUIRE_NOTHROW(to_storage.create(to_memory, 4, 16));

    std::default_random_engine rng;
    for (int i = 0; i < 16; ++i) {
        block b(i, 4);
        *(uint32_t *) b.data.data() = (uint32_t) rng();
        to_storage.set_block(b);
    }


    block_storage_sync sync(from_storage, to_storage);
    sync.synchronise();

    from_memory->seekg(0, std::ios_base::end);
    auto from_memory_length = from_memory->tellg();
    from_memory->seekg(0, std::ios_base::beg);

    to_memory->seekg(0, std::ios_base::end);
    auto to_memory_length = to_memory->tellg();
    to_memory->seekg(0, std::ios_base::beg);

    REQUIRE(from_memory_length == to_memory_length);

    buffer from_buffer(from_memory_length, 0);
    buffer to_buffer(to_memory_length, 0);

    from_memory->read((char *) from_buffer.data(), from_memory_length);
    to_memory->read((char *) to_buffer.data(), to_memory_length);

//    hex_dump(from_buffer.data(), from_buffer.size(), std::cout, 32);
//    hex_dump(to_buffer.data(), to_buffer.size(), std::cout, 32);

    REQUIRE(std::memcmp(from_buffer.data(), to_buffer.data(), from_memory_length) == 0);
}