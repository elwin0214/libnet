SET (HTTP_SRCS http_server.cc http_response.cc http_processor.cc)
SET (HTTP_HEADERS http_context.h http_request.h http_server.h)

message(" PROJECT_SOURCE_DIR = " ${PROJECT_SOURCE_DIR})
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})


ADD_LIBRARY(libnet_http SHARED ${HTTP_SRCS})
TARGET_LINK_LIBRARIES(libnet_http libnet)

SET_TARGET_PROPERTIES(libnet_http PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(libnet_http PROPERTIES VERSION 1.0 SOVERSION 1)


INSTALL(TARGETS libnet_http LIBRARY DESTINATION lib)
INSTALL(FILES ${HEADERS} DESTINATION include/libnet/http)

#add_executable(test_httpserver test/test_httpserver.cc)
#target_link_libraries(test_httpserver libnet_http libnet)

add_subdirectory(test)