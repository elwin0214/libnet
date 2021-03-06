add_executable(buffer_unitest buffer_unitest.cc)
target_link_libraries(buffer_unitest libnet ${GTEST_LIBRARY})

add_executable(endian_unitest endian_unitest.cc)
target_link_libraries(endian_unitest libnet ${GTEST_LIBRARY})

add_executable(inet_address_unitest inet_address_unitest.cc)
target_link_libraries(inet_address_unitest libnet ${GTEST_LIBRARY})

add_executable(logger_stream_unitest logger_stream_unitest.cc)
target_link_libraries(logger_stream_unitest libnet ${GTEST_LIBRARY})

add_executable(logger_unitest logger_unitest.cc)
target_link_libraries(logger_unitest libnet ${GTEST_LIBRARY})

add_executable(logger_test logger_test.cc)
target_link_libraries(logger_test libnet)

add_executable(timerqueue_unitest timerqueue_unitest.cc)
target_link_libraries(timerqueue_unitest libnet ${GTEST_LIBRARY})


add_executable(thread_unitest thread_unitest.cc)
target_link_libraries(thread_unitest libnet ${GTEST_LIBRARY})

add_executable(timestamp_unitest timestamp_unitest.cc)
target_link_libraries(timestamp_unitest libnet)

add_executable(client_unitest client_unitest.cc)
target_link_libraries(client_unitest libnet ${GTEST_LIBRARY})

add_executable(server_unitest server_unitest.cc)
target_link_libraries(server_unitest libnet ${GTEST_LIBRARY})

add_executable(countdown_latch_unitest countdown_latch_unitest.cc)
target_link_libraries(countdown_latch_unitest libnet)

add_executable(digits_unitest digits_unitest.cc)
target_link_libraries(digits_unitest libnet  ${GTEST_LIBRARY})

add_executable(condition_unitest condition_unitest.cc)
target_link_libraries(condition_unitest libnet ${GTEST_LIBRARY})

add_executable(eventloop_unitest eventloop_unitest.cc)
target_link_libraries(eventloop_unitest libnet ${GTEST_LIBRARY})

add_test(NAME buffer_unitest COMMAND buffer_unitest)
add_test(NAME endian_unitest COMMAND endian_unitest)
add_test(NAME inet_address_unitest COMMAND inet_address_unitest)
add_test(NAME logger_stream_unitest COMMAND logger_stream_unitest)
add_test(NAME logger_unitest COMMAND logger_unitest)
add_test(NAME timerqueue_unitest COMMAND timerqueue_unitest)
add_test(NAME thread_unitest COMMAND thread_unitest)
add_test(NAME timestamp_unitest COMMAND timestamp_unitest)
add_test(NAME countdown_latch_unitest COMMAND countdown_latch_unitest)
add_test(NAME digits_unitest COMMAND digits_unitest)
add_test(NAME condition_unitest COMMAND condition_unitest)
add_test(NAME eventloop_unitest COMMAND eventloop_unitest)
add_test(NAME server_unitest COMMAND server_unitest)
add_test(NAME client_unitest COMMAND client_unitest)