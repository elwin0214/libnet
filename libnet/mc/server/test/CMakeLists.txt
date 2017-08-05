if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
message("add PROFILE")
set_source_files_properties(server/test/memcached_server_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcache_prof.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()



if(BENCHMARK_LIBEARY)
add_executable(memcache_bench ../allocator.cc ../slab.cc ../lru.cc ../hashtable.cc ../mem_cache.cc memcache_bench.cc )
TARGET_LINK_LIBRARIES(memcache_bench libnet ${BENCHMARK_LIBEARY})
message(" memcache_bench link benchmark")
endif()

add_executable(memcached_server_bench ../allocator.cc ../slab.cc ../lru.cc ../tokenizer.cc ../hashtable.cc ../mem_cache.cc ../mem_handler.cc ../../msg/reader.cc ../../msg/request_codec.cc ../../msg/response_codec.cc ../mem_server.cc memcached_server_bench.cc)

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(memcached_server_bench libnet profiler)
else()
TARGET_LINK_LIBRARIES(memcached_server_bench libnet)
endif()

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
add_executable(memcache_prof ../allocator.cc ../slab.cc ../lru.cc ../hashtable.cc ../mem_cache.cc memcache_prof.cc)
TARGET_LINK_LIBRARIES(memcache_prof libnet profiler)
endif()

add_executable(memcache_unitest ../allocator.cc ../slab.cc ../lru.cc ../hashtable.cc ../mem_cache.cc memcache_unitest.cc )
TARGET_LINK_LIBRARIES(memcache_unitest libnet ${GTEST_LIBRARY})

add_executable(hashtable_unitest ../allocator.cc ../slab.cc ../hashtable.cc hashtable_unitest.cc)
TARGET_LINK_LIBRARIES(hashtable_unitest libnet ${GTEST_LIBRARY})

add_executable(lru_unitest ../lru.cc lru_unitest.cc)
TARGET_LINK_LIBRARIES(lru_unitest libnet ${GTEST_LIBRARY})

add_executable(slab_unitest ../allocator.cc ../slab.cc slab_unitest.cc)
TARGET_LINK_LIBRARIES(slab_unitest libnet ${GTEST_LIBRARY})

add_executable(tokenizer_unitest ../tokenizer.cc tokenizer_unitest.cc)
TARGET_LINK_LIBRARIES(tokenizer_unitest libnet ${GTEST_LIBRARY})

add_executable(allocator_unitest ../allocator.cc allocator_unitest.cc)
TARGET_LINK_LIBRARIES(allocator_unitest libnet ${GTEST_LIBRARY})


add_test(NAME hashtable_unitest COMMAND hashtable_unitest)
add_test(NAME lru_unitest COMMAND lru_unitest)
add_test(NAME slab_unitest COMMAND slab_unitest)
add_test(NAME tokenizer_unitest COMMAND tokenizer_unitest)
add_test(NAME memcache_unitest COMMAND memcache_unitest)
add_test(NAME allocator_unitest COMMAND allocator_unitest)
