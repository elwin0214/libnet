add_executable(command_unitest client/command.cc client/test/command_unitest.cc)
TARGET_LINK_LIBRARIES(command_unitest libnet ${GTEST_LIBRARY})

add_executable(memcached_client_test client/command.cc client/memcached_client.cc client/test/memcached_client_test.cc)
TARGET_LINK_LIBRARIES(memcached_client_test libnet)


if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
set_source_files_properties(client/test/memcached_client_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcache_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcached_server_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(server/test/memcache_prof.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()

add_executable(memcached_client_bench client/command.cc client/memcached_client.cc client/test/memcached_client_bench.cc)

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(memcached_client_bench libnet profiler)
else()
TARGET_LINK_LIBRARIES(memcached_client_bench libnet)
endif()


if(BENCHMARK_LIBEARY)
add_executable(memcache_bench server/allocator.cc server/slab.cc server/lru.cc server/hashtable.cc server/memcache.cc server/test/memcache_bench.cc )
TARGET_LINK_LIBRARIES(memcache_bench libnet ${BENCHMARK_LIBEARY})
message(" memcache_bench link benchmark")
endif()

add_executable(memcached_server_bench server/allocator.cc server/slab.cc server/lru.cc server/tokenizer.cc server/processor.cc server/hashtable.cc server/memcache.cc server/memcached_server.cc server/test/memcached_server_bench.cc )

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

add_executable(processor_unitest server/tokenizer.cc server/processor.cc server/test/processor_unitest.cc)
TARGET_LINK_LIBRARIES(processor_unitest libnet ${GTEST_LIBRARY})

add_executable(slab_unitest server/allocator.cc server/slab.cc server/test/slab_unitest.cc)
TARGET_LINK_LIBRARIES(slab_unitest libnet ${GTEST_LIBRARY})

add_executable(tokenizer_unitest server/tokenizer.cc server/test/tokenizer_unitest.cc ${GTEST_LIBRARY})
TARGET_LINK_LIBRARIES(tokenizer_unitest libnet ${GTEST_LIBRARY})

 


add_test(NAME command_unitest COMMAND command_unitest)
add_test(NAME hashtable_unitest COMMAND hashtable_unitest)
add_test(NAME lru_unitest COMMAND lru_unitest)
add_test(NAME processor_unitest COMMAND processor_unitest)
add_test(NAME slab_unitest COMMAND slab_unitest)
add_test(NAME tokenizer_unitest COMMAND tokenizer_unitest)
add_test(NAME memcache_unitest COMMAND memcache_unitest)

