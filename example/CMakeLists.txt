INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

add_executable(echo_server_test echo/echo_server.cc echo/echo_server_test.cc)
TARGET_LINK_LIBRARIES(echo_server_test libnet)


add_executable(command_unitest memcached/command.cc memcached/test/command_unitest.cc)

add_executable(memcached_client_test memcached/command.cc memcached/memcached_client.cc memcached/test/memcached_client_test.cc)

#message(STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
#if(CMAKE_BUILD_TYPE MATCHES "PROFILER")
#if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
#  set_source_files_properties(memcached/test/memcached_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILER")
#  add_executable(memcached_bench_profiler memcached/command.cc memcached/memcached_client.cc memcached/test/memcached_bench.cc)
#  target_link_libraries(memcached_bench_profiler libnet tcmalloc profiler)
#endif()


add_executable(memcached_bench memcached/command.cc memcached/memcached_client.cc memcached/test/memcached_bench.cc)
TARGET_LINK_LIBRARIES(memcached_bench libnet)


#set_source_files_properties(memcached/test/cpuprof_test.cc PROPERTIES COMPILE_FLAGS "-DPROFILER")
#add_executable(cpuprof_test memcached/test/cpuprof_test.cc)
#TARGET_LINK_LIBRARIES(cpuprof_test profiler)


 
TARGET_LINK_LIBRARIES(command_unitest libnet)
TARGET_LINK_LIBRARIES(memcached_client_test libnet)
