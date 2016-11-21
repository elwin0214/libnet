#include <iostream>
#include <vector>
#include <iostream>
#include <libnet/logger.cc>
#include <libnet/buffer.h>
#include <libnet/cstring.h>
#include <libnet/http/http_request.h>
#include <libnet/http/http_context.h>
#include <libnet/digits.h>
#include <libnet/http/http_processor.h>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;
using namespace libnet::http;
using namespace std::placeholders;

TEST(HttpProcessor, GetRequest)
{
  HttpProcessor processor;
  HttpContext context;

  processor.setBodyReaderCallBack([](const Buffer& buffer, size_t allowed, HttpContext& ctx){
    return allowed;
  });

  processor.setRequestHandlerCallBack([](const Buffer& buffer, HttpContext& ctx){
    HttpResponse& response = ctx.getResponse();
    response.setStatus(HttpResponse::OK);
    response.send("hello world", 11);
    response.flush();
  });

  Buffer respBuffer;
  context.getResponse().setSendCallback([&respBuffer](const CString& cstring){
    respBuffer.append(cstring.data(), cstring.length());
  });

  Buffer buffer(1024);
  buffer.append("GET / HTTP/1.1\r\n\r\n");
  context.getResponse().setClose(true);
  processor.process(buffer, context);
  ASSERT_TRUE(context.getState() == HttpContext::kBodySent);

  std::string resp ="HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello world";
  ASSERT_EQ(resp, respBuffer.toString());
}

TEST(HttpProcessor, KeepAlive)
{
  HttpProcessor processor;
  HttpContext context;

  processor.setBodyReaderCallBack([](const Buffer& buffer, size_t allowed, HttpContext& ctx){
    return allowed;
  });

  processor.setRequestHandlerCallBack([](const Buffer& buffer, HttpContext& ctx){
    HttpResponse& response = ctx.getResponse();
    response.setStatus(HttpResponse::OK);
    response.send("hello world", 11);
    response.flush();
  });

  Buffer respBuffer;
  context.getResponse().setSendCallback([&respBuffer](const CString& cstring){
    respBuffer.append(cstring.data(), cstring.length());
  });

  Buffer buffer(1024);
  buffer.append("GET / HTTP/1.1\r\n\r\n");
  context.getResponse().setClose(false);
  processor.process(buffer, context);
  ASSERT_TRUE(context.getState() == HttpContext::kBodySent);

  std::string resp ="HTTP/1.1 200 OK\r\nContent-Length: 11\r\nConnection: Keep-Alive\r\n\r\nhello world";
  ASSERT_EQ(resp, respBuffer.toString());
}
 
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}