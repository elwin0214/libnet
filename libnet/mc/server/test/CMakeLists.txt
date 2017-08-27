if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
message("add PROFILE")
set_source_files_properties(memcached_server_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(memcache_prof.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()

if(BENCHMARK_LIBEARY)
add_executable(memcache_bench memcache_bench.cc )
TARGET_LINK_LIBRARIES(memcache_bench libnet libnet_mc ${BENCHMARK_LIBEARY})
message(" memcache_bench link benchmark")
endif()

add_executable(memcached_server_bench memcached_server_bench.cc)

if(BENCHMARK_LIBEARY)
add_executable(htable_bench  htable_bench.cc ../item.cc)
TARGET_LINK_LIBRARIES(htable_bench libnet ${BENCHMARK_LIBEARY})
message(" htable_bench link benchmark")
endif()


if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(memcached_server_bench libnet libnet_mc profiler)
else()
TARGET_LINK_LIBRARIES(memcached_server_bench libnet libnet_mc)
endif()

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
add_executable(memcache_prof ../allocator.cc ../slab.cc ../lru.cc ../item.cc ../mem_cache.cc memcache_prof.cc)
TARGET_LINK_LIBRARIES(memcache_prof libnet profiler)
endif()

add_executable(memcache_unitest ../allocator.cc ../slab.cc ../lru.cc ../item.cc ../mem_cache.cc memcache_unitest.cc )
TARGET_LINK_LIBRARIES(memcache_unitest libnet ${GTEST_LIBRARY})

add_executable(htable_unitest ../allocator.cc ../slab.cc ../item.cc  htable_unitest.cc)
TARGET_LINK_LIBRARIES(htable_unitest libnet ${GTEST_LIBRARY})

add_executable(lru_unitest ../lru.cc ../item.cc lru_unitest.cc)
TARGET_LINK_LIBRARIES(lru_unitest libnet ${GTEST_LIBRARY})

add_executable(slab_unitest ../allocator.cc ../item.cc ../slab.cc slab_unitest.cc)
TARGET_LINK_LIBRARIES(slab_unitest libnet ${GTEST_LIBRARY})

add_executable(tokenizer_unitest ../tokenizer.cc ../item.cc tokenizer_unitest.cc)
TARGET_LINK_LIBRARIES(tokenizer_unitest libnet ${GTEST_LIBRARY})

add_executable(allocator_unitest ../allocator.cc ../item.cc allocator_unitest.cc)
TARGET_LINK_LIBRARIES(allocator_unitest libnet ${GTEST_LIBRARY})


add_test(NAME htable_unitest COMMAND htable_unitest)
add_test(NAME lru_unitest COMMAND lru_unitest)
add_test(NAME slab_unitest COMMAND slab_unitest)
add_test(NAME tokenizer_unitest COMMAND tokenizer_unitest)
add_test(NAME memcache_unitest COMMAND memcache_unitest)
add_test(NAME allocator_unitest COMMAND allocator_unitest)
