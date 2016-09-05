add_executable(command_unitest client/command.cc client/test/command_unitest.cc)
TARGET_LINK_LIBRARIES(command_unitest libnet)

add_executable(memcached_client_test client/command.cc client/memcached_client.cc client/test/memcached_client_test.cc)
TARGET_LINK_LIBRARIES(memcached_client_test libnet)


if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
set_source_files_properties(client/test/memcached_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()

add_executable(memcached_bench client/command.cc client/memcached_client.cc client/test/memcached_bench.cc)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY AND PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(memcached_bench libnet tcmalloc profiler)
else()
TARGET_LINK_LIBRARIES(memcached_bench libnet)
endif()


add_executable(slab_unitest server/allocator.cc server/slab.cc server/test/slab_unitest.cc)
TARGET_LINK_LIBRARIES(slab_unitest libnet)

add_executable(hashtable_unitest server/allocator.cc server/slab.cc server/hashtable.cc server/test/hashtable_unitest.cc)
TARGET_LINK_LIBRARIES(hashtable_unitest libnet)

add_executable(tokenizer_unitest server/tokenizer.cc server/test/tokenizer_unitest.cc)
TARGET_LINK_LIBRARIES(tokenizer_unitest libnet)

add_executable(processor_unitest server/tokenizer.cc server/processor.cc server/test/processor_unitest.cc)
TARGET_LINK_LIBRARIES(processor_unitest libnet)


add_executable(memcached_server_test server/allocator.cc server/slab.cc server/tokenizer.cc server/processor.cc server/hashtable.cc server/memcached_server.cc server/test/memcached_server_test.cc)
TARGET_LINK_LIBRARIES(memcached_server_test libnet)


