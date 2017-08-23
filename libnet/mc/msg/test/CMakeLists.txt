add_executable(codec_unitest ../reader.cc ../request_codec.cc ../response_codec.cc codec_unitest.cc)
TARGET_LINK_LIBRARIES(codec_unitest libnet ${GTEST_LIBRARY})

add_executable(reader_unitest ../reader.cc  reader_unitest.cc)
TARGET_LINK_LIBRARIES(reader_unitest libnet ${GTEST_LIBRARY})

if(BENCHMARK_LIBEARY)
add_executable(codec_bench  codec_bench.cc)
TARGET_LINK_LIBRARIES(codec_bench libnet libnet_mc ${GTEST_LIBRARY} ${BENCHMARK_LIBEARY})
message(" codec_bench link benchmark")
endif()


add_test(NAME reader_unitest COMMAND reader_unitest)
add_test(NAME codec_unitest COMMAND codec_unitest)