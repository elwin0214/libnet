INCLUDE(CheckFunctionExists)
check_function_exists(epoll_create HAVE_EPOLL)

if(HAVE_EPOLL)
  set_source_files_properties(selector/selector_provider.cc PROPERTIES COMPILE_FLAGS "-DHAVE_POLL")
endif()

SET (SRCS
 acceptor.cc
 buffer.cc
 channel.cc
 client.cc
 connection.cc
 connector.cc
 eventloop.cc
 eventloop_group.cc
 eventloop_thread.cc
 inet_address.cc
 logger.cc
 server.cc
 socket.cc
 socket_ops.cc
 thread.cc
 thread_pool.cc
 timer.cc
 timer_queue.cc
 timestamp.cc
 selector/selector_provider.cc
 selector/selector.cc
 selector/default_selector.cc
 selector/poll_selector.cc
)

if(HAVE_EPOLL)
  message(STATUS "library include epoll")
  SET(SRCS ${SRCS} selector/epoll_selector.cc)
endif()

message(STATUS " source files = " ${SRCS})

SET (HEADERS
  buffer.h
  channel.h
  client.h
  condition.h
  connection.h
  cstring.h
  current_thread.h
  eventloop.h
  eventloop_thread.h
  inet_address.h
  logger.h
  mutexlock.h
  nocopyable.h
  server.h
  socket.h
  thread.h
  thread_pool.h
  timer.h
  timer_queue.h
  timestamp.h
)

message("PROJECT_SOURCE_DIR = " ${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

ADD_LIBRARY(libnet SHARED ${SRCS})
SET_TARGET_PROPERTIES(libnet PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(libnet PROPERTIES VERSION 1.0 SOVERSION 1)

#ADD_LIBRARY(libnet_static STATIC ${SRCS})
#SET_TARGET_PROPERTIES(libnet_static PROPERTIES OUTPUT_NAME "libnet")
#SET_TARGET_PROPERTIES(libnet_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(libnet_static PROPERTIES VERSION 1.0 SOVERSION 1)

INSTALL(TARGETS libnet DESTINATION lib)
INSTALL(FILES ${HEADERS} DESTINATION include/libnet)

add_subdirectory(http)
add_subdirectory(test)
