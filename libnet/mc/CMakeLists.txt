SET (MC_SERVER_SRCS server/mem_server.cc server/allocator.cc server/hashtable.cc server/lru.cc server/mem_handler.cc server/mem_cache.cc server/slab.cc server/tokenizer.cc)
SET (MC_CLIENT_SRCS client/async_client.cc client/req_cache.cc client/session.cc)
#SET (MC_PROXY_SRCS proxy/back_client.cc proxy/front_server.cc proxy/proxy_server.cc)
SET (MC_MSG_SRCS msg/reader.cc msg/request_codec.cc msg/response_codec.cc)

SET (MC_SEVER_HEADERS allocator.h mem_handler.h hashtable.h item.h lru.h mem_server.h mem_cache.h slab.h tokenizer.h)
SET (MC_CLIENT_HEADERS async_client.h command.h session.h)
#SET (MC_PROXY_HEADERS proxy_server.h)
SET (MC_MSG_HEADERS mcode.h message.h reader.h request_codec.h response_codec.h)

SET(MC_SRCS ${MC_SERVER_SRCS} ${MC_CLIENT_SRCS} ${MC_MSG_SRCS})
SET(MC_HEADERS ${MC_SEVER_HEADERS} ${MC_CLIENT_HEADERS} ${MC_MSG_HEADERS})

message(" PROJECT_SOURCE_DIR = " ${PROJECT_SOURCE_DIR})

ADD_LIBRARY(libnet_mc SHARED ${MC_SRCS})
TARGET_LINK_LIBRARIES(libnet_mc libnet)

SET_TARGET_PROPERTIES(libnet_mc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(libnet_mc PROPERTIES VERSION 1.0 SOVERSION 1)

INSTALL(TARGETS libnet_mc LIBRARY DESTINATION lib)
INSTALL(FILES ${MC_HEADERS} DESTINATION include/libnet/mc)

add_subdirectory(msg/test)
add_subdirectory(server/test)
add_subdirectory(client/test)