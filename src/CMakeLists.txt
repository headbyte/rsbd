
project(rsbd)

include_directories(../lib/stduuid/include)
include_directories(../lib/args)

set(RSBD_LIBRARY_SOURCE block.cpp binary.h block_storage.h block_storage_sync.h)


add_library(rsbd-lib ${RSBD_LIBRARY_SOURCE})
target_link_libraries(rsbd-lib libcryptopp.a)


include(FetchContent)
FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.25.0
)
FetchContent_MakeAvailable(gRPC)

add_executable(rsbd rsbd.cpp)
target_link_libraries(rsbd rsbd-lib grpc++)
add_dependencies(rsbd rsbd-lib)