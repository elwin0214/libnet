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

TEST(HttpProcessor, Post)
{
  HttpProcessor processor;
  HttpContext context;
  Buffer reqBuffer;

  processor.setBodyReaderCallBack([&reqBuffer](const Buffer& buffer, size_t allowed, HttpContext& ctx){
    reqBuffer.append(buffer.beginRead(), allowed);
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
  buffer.append("POST / HTTP/1.1\r\n");
  buffer.append("Content-Length: 3\r\n");
  buffer.append("\r\n");
  ASSERT_TRUE(processor.process(buffer, context));
  ASSERT_TRUE(context.getState() == HttpContext::kBodySending);
  buffer.append("12");

  processor.process(buffer, context);
  ASSERT_TRUE(context.getState() == HttpContext::kBodySending);

  buffer.append("3");
  processor.process(buffer, context);
  ASSERT_TRUE(context.getState() == HttpContext::kBodySent);

  ASSERT_EQ("123", reqBuffer.toString());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}