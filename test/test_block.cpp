
#include <catch2/catch.hpp>
#include <sstream>
#include <iostream>
#include "block.h"

using namespace rsbd;


TEST_CASE("single block storage header test", "[single]") {
    single_file_block_storage_index header;
    header.init(2);
    header.starting_block = 0;
    header.block_size = 1;

    std::stringstream str;
    header.serialize(str);

    str.seekg(0, std::ios_base::end);

    single_file_block_storage_index header2;
    header2.reverse_deserialize(str);

    REQUIRE(header.starting_block == header2.starting_block);
    REQUIRE(header.block_size == header2.block_size);
    REQUIRE(header.block_count == header2.block_count);

    REQUIRE(std::memcmp(header.magic, header2.magic, 4) == 0);
    REQUIRE(std::memcmp(header.block_map.data(), header2.block_map.data(), header.block_map.size()) == 0);
    REQUIRE(header.hashes == header2.hashes);
}

TEST_CASE("single block storage test", "[single]") {

    SECTION("create a single file block storage") {
        {
            single_file_block_storage sfbs;
            sfbs.create("/tmp/disk0.bin", 1, 8);

            for (int i = 0; i < 8; ++i) {
                block b;
                b.id = i;
                b.size = 1;
                b.data.push_back(i);
                sfbs.set_block(b);
            }
        }

        {

            single_file_block_storage sfbs;
            sfbs.open("/tmp/disk0.bin");

            REQUIRE(sfbs.get_block_count() == 8);
            REQUIRE(sfbs.get_block_size(0) == 1);
            // TODO: remove get_block_size id parameter since it is better to have all blocks same size

            for (size_t i = 0; i < sfbs.get_block_count(); i++) {
                REQUIRE(sfbs.has_block(i) == true);

                block b;
                REQUIRE (sfbs.get_block(i, b) == true);
                REQUIRE(b.size == 1);
                REQUIRE(b.id == i);
                REQUIRE(b.data[0] == i);

                auto hash = b.get_hash();
                auto hash2 = sfbs.get_block_hash_from_header(i);

                REQUIRE((hash == hash2) == true);
            }

        }

    }


}