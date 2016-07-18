INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

add_executable(echo_server_test echo/echo_server.cc echo/echo_server_test.cc)
TARGET_LINK_LIBRARIES(echo_server_test libnet)


add_executable(memcached_client_test memcached/command.cc memcached/memcached_client.cc memcached/memcached_client_test.cc)
TARGET_LINK_LIBRARIES(memcached_client_test libnet)