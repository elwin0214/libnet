add_executable(codec_unitest message/reader.cc message/request_codec.cc message/response_codec.cc message/test/codec_unitest.cc)
TARGET_LINK_LIBRARIES(codec_unitest libnet ${GTEST_LIBRARY})

add_executable(reader_unitest message/reader.cc  message/test/reader_unitest.cc)
TARGET_LINK_LIBRARIES(reader_unitest libnet ${GTEST_LIBRARY})

#add_executable(command_unitest client/test/command_unitest.cc)
#TARGET_LINK_LIBRARIES(command_unitest libnet ${GTEST_LIBRARY}) 

add_executable(async_mclient_test message/reader.cc message/request_codec.cc message/response_codec.cc client/async_client.cc client/test/async_client_test.cc)
TARGET_LINK_LIBRARIES(async_mclient_test libnet)


if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
message("add PROFILE")
set_source_files_properties(client/test/async_client_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(client/test/mclient_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")

#set_source_files_properties(server/test/memcache_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcached_server_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcache_prof.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()

add_executable(async_mclient_bench message/reader.cc message/request_codec.cc message/response_codec.cc client/async_client.cc client/test/async_client_bench.cc)

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(async_mclient_bench libnet profiler)
else()
TARGET_LINK_LIBRARIES(async_mclient_bench libnet)
endif()

add_executable(mclient_bench message/reader.cc message/response_codec.cc client/test/client_bench.cc)

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(mclient_bench libnet profiler)
else()
TARGET_LINK_LIBRARIES(mclient_bench libnet)
endif()


if(BENCHMARK_LIBEARY)
add_executable(memcache_bench server/allocator.cc server/slab.cc server/lru.cc server/hashtable.cc server/memcache.cc server/test/memcache_bench.cc )
TARGET_LINK_LIBRARIES(memcache_bench libnet ${BENCHMARK_LIBEARY})
message(" memcache_bench link benchmark")
endif()

add_executable(memcached_server_bench server/allocator.cc server/slab.cc server/lru.cc server/tokenizer.cc server/hashtable.cc server/memcache.cc server/memhandler.cc message/reader.cc message/request_codec.cc message/response_codec.cc server/memcached_server.cc server/test/memcached_server_bench.cc )

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(memcached_server_bench libnet profiler)
else()
TARGET_LINK_LIBRARIES(memcached_server_bench libnet)
endif()

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
add_executable(memcache_prof server/allocator.cc server/slab.cc server/lru.cc server/hashtable.cc server/memcache.cc server/test/memcache_prof.cc)
TARGET_LINK_LIBRARIES(memcache_prof libnet profiler)
endif()

add_executable(memcache_unitest server/allocator.cc server/slab.cc server/lru.cc server/hashtable.cc server/memcache.cc server/test/memcache_unitest.cc )
TARGET_LINK_LIBRARIES(memcache_unitest libnet ${GTEST_LIBRARY})

add_executable(hashtable_unitest server/allocator.cc server/slab.cc server/hashtable.cc server/test/hashtable_unitest.cc)
TARGET_LINK_LIBRARIES(hashtable_unitest libnet ${GTEST_LIBRARY})

add_executable(lru_unitest server/lru.cc server/test/lru_unitest.cc)
TARGET_LINK_LIBRARIES(lru_unitest libnet ${GTEST_LIBRARY})


add_executable(slab_unitest server/allocator.cc server/slab.cc server/test/slab_unitest.cc)
TARGET_LINK_LIBRARIES(slab_unitest libnet ${GTEST_LIBRARY})

add_executable(tokenizer_unitest server/tokenizer.cc server/test/tokenizer_unitest.cc)
TARGET_LINK_LIBRARIES(tokenizer_unitest libnet ${GTEST_LIBRARY})

add_executable(allocator_unitest server/allocator.cc server/test/allocator_unitest.cc)
TARGET_LINK_LIBRARIES(allocator_unitest libnet ${GTEST_LIBRARY})


add_test(NAME reader_unitest COMMAND reader_unitest)
add_test(NAME codec_unitest COMMAND codec_unitest)

add_test(NAME hashtable_unitest COMMAND hashtable_unitest)
add_test(NAME lru_unitest COMMAND lru_unitest)
add_test(NAME slab_unitest COMMAND slab_unitest)
add_test(NAME tokenizer_unitest COMMAND tokenizer_unitest)
add_test(NAME memcache_unitest COMMAND memcache_unitest)
add_test(NAME allocator_unitest COMMAND allocator_unitest)