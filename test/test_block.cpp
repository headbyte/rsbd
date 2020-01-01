
#include <catch2/catch.hpp>
#include <sstream>
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

}