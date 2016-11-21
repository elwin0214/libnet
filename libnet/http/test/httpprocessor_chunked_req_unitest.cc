#include <iostream>
#include <vector>
#include <iostream>
#include <string>
#include <libnet/logger.cc>
#include <libnet/buffer.h>
#include <libnet/http/http_request.h>
#include <libnet/http/http_context.h>
#include <libnet/digits.h>
#include <libnet/http/http_processor.h>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;
using namespace libnet::http;
using namespace std::placeholders;

bool gClose = false;

// process request
struct ProxyHandler
{
  ProxyHandler(Buffer* buffer)
    : requestBodyBuffer_(buffer)
  {

  }

  size_t handleBody(const Buffer& buffer, size_t allowed, HttpContext& context)
  {
    requestBodyBuffer_->append(buffer.beginRead(), allowed);
    return allowed;
  }

  void handleRequest(const Buffer& buffer, HttpContext& context)
  {
    HttpResponse& response = context.getResponse();
    response.setStatus(HttpResponse::OK);
    response.setClose(false);
    response.setChunked(true);

    response.send("123456", 6);
    response.flush();

    response.send("012345678901", 12);
    response.flush();

    response.send("0123456789", 10);
    response.send("0123456789", 10);
    response.flush();

    response.finish();
  }

  Buffer* requestBodyBuffer_;
};

//mock connection
struct ProxyConnection
{
  ProxyConnection(Buffer* buffer)
    : responseBuffer_(buffer)
  {

  }

  void sendString(CString cstring)
  {
    responseBuffer_->append(cstring.data(), cstring.length());
  }

  Buffer* responseBuffer_;
};

class ChunkedHttpProcessorTest : public ::testing::Test
{
protected:

  virtual void SetUp()
  {
    requestBodyBuffer_ = new Buffer();
    responseBuffer_ = new Buffer();

    proxyConnection_ = new ProxyConnection(responseBuffer_);
    proxyHandler_ = new ProxyHandler(requestBodyBuffer_);
    httpProcessor_ = new HttpProcessor();
    context_ = new HttpContext();

    httpProcessor_->setBodyReaderCallBack(std::bind(&ProxyHandler::handleBody, proxyHandler_, _1, _2, _3));
    httpProcessor_->setRequestHandlerCallBack(std::bind(&ProxyHandler::handleRequest, proxyHandler_, _1, _2));
    context_->getResponse().setSendCallback(std::bind(&ProxyConnection::sendString, proxyConnection_, _1));
  }

  virtual void TearDown()
  {
    delete proxyConnection_;
    delete proxyHandler_;
    delete httpProcessor_;
    delete context_;
    delete requestBodyBuffer_;
    delete responseBuffer_;
  }

  ProxyConnection* proxyConnection_;
  ProxyHandler* proxyHandler_;
  HttpProcessor* httpProcessor_;
  HttpContext* context_;
  Buffer* requestBodyBuffer_;
  Buffer* responseBuffer_;

};

TEST_F(ChunkedHttpProcessorTest, test)
{
  Buffer buffer(0, 1024);
  buffer.append("POST / HTTP/1.1\r\n");

  ASSERT_TRUE(httpProcessor_->process(buffer, *context_));
  ASSERT_TRUE(context_->getState() == HttpContext::kRequestLineProcessed);
  
  buffer.append("Transfer-Encoding: chunked\r\n");
  buffer.append("\r\n");

  ASSERT_TRUE(httpProcessor_->process(buffer, *context_));
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkSizeSending);
  string encoding = context_->getRequest().getHeader("Transfer-Encoding");
  ASSERT_TRUE(encoding == "chunked");

  buffer.append("3");
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkSizeSending);

  buffer.append("\r\n12");  //got size
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkBodySending);

  buffer.append("3\r\n");  //got body
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkSizeSending);

  buffer.append("a\r");  //
  ASSERT_TRUE(httpProcessor_->process(buffer, *context_));
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkSizeSending);

  buffer.append("\n");   //got size
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkBodySending);

  buffer.append("0123456789\r\n"); //got body
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kPartChunkSizeSending);

  buffer.append("0\r\n\r\n");
  httpProcessor_->process(buffer, *context_);
  ASSERT_TRUE(context_->getState() == HttpContext::kAllChunkSent);

  ASSERT_EQ ("1230123456789", requestBodyBuffer_->toString());

  const char* chunckedPtr = responseBuffer_->find("Transfer-Encoding: chunked\r\n");
  ASSERT_TRUE( NULL != chunckedPtr);

  const char* bodyPtr = responseBuffer_->find("\r\n\r\n");
  bodyPtr += 4;
  std::string body = std::string(bodyPtr, responseBuffer_->beginWrite() - bodyPtr);

  ASSERT_EQ (body, "6\r\n123456\r\nc\r\n012345678901\r\n14\r\n01234567890123456789\r\n0\r\n\r\n");
}

int main(int argc, char **argv)
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

 