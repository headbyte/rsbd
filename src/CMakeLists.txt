
project (rsbd)

include_directories(../lib/stduuid/include)

set(RSBD_LIBRARY_SOURCE block.cpp binary.h block_storage.h block_storage_sync.h)

add_executable(rsbd rsbd.cpp ${RSBD_LIBRARY_SOURCE})



add_library(rsbd-lib ${RSBD_LIBRARY_SOURCE})
target_link_libraries(rsbd-lib libcryptopp.a)