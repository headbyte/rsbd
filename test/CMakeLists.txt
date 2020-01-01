project (rsbd-test)

include_directories(../lib/catch2/single_include)
include_directories(../src)

add_executable(rsbd-test test_block.cpp main.cpp)
target_link_libraries(rsbd-test rsbd-lib)
add_dependencies(rsbd-test rsbd-lib)