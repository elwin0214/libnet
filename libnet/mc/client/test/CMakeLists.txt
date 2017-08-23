add_executable(async_client_test async_client_test.cc)
TARGET_LINK_LIBRARIES(async_client_test libnet libnet_mc ${GTEST_LIBRARY})

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
message("add PROFILE in memcached client ")
set_source_files_properties(async_client_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
set_source_files_properties(client_bench.cc PROPERTIES COMPILE_FLAGS "-DPROFILE")
endif()


add_executable(async_client_bench async_client_bench.cc)
if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(async_client_bench libnet libnet_mc profiler)
else()
TARGET_LINK_LIBRARIES(async_client_bench libnet libnet_mc)
endif()

add_executable(client_bench client_bench.cc)

if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
TARGET_LINK_LIBRARIES(client_bench libnet libnet_mc profiler)
else()
TARGET_LINK_LIBRARIES(client_bench libnet libnet_mc)
endif()

add_executable(session_unitest session_unitest.cc)
TARGET_LINK_LIBRARIES(session_unitest libnet libnet_mc ${GTEST_LIBRARY})

add_test(NAME session_unitest COMMAND session_unitest)