add_executable(codec_unitest ../reader.cc ../request_codec.cc ../response_codec.cc codec_unitest.cc)
TARGET_LINK_LIBRARIES(codec_unitest libnet ${GTEST_LIBRARY})

add_executable(reader_unitest ../reader.cc  reader_unitest.cc)
TARGET_LINK_LIBRARIES(reader_unitest libnet ${GTEST_LIBRARY})

add_test(NAME reader_unitest COMMAND reader_unitest)
add_test(NAME codec_unitest COMMAND codec_unitest)