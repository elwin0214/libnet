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

using namespace std;
using namespace libnet;
using namespace libnet::http;
using namespace std::placeholders;

HttpProcessor* gHttpProcessor = NULL;
HttpContext* gContext = NULL;

Buffer* gRequestBodyBuffer = NULL;
Buffer* gResponseBuffer = NULL;

bool gClose = false;

// process request
struct ProxyHandler
{
  size_t handleBody(const Buffer& buffer, size_t allowed, HttpContext& context)
  {
    gRequestBodyBuffer->append(buffer.beginRead(), allowed);
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
};

//mock connection
struct ProxyConnection
{

  void sendString(CString cstring)
  {
    gResponseBuffer->append(cstring.data(), cstring.length());
  }

};

ProxyHandler proxyHandler;
ProxyConnection proxyConnection;


struct TestChunkedHttpProcessor
{
  virtual void setup()
  {
    gHttpProcessor = new HttpProcessor();
    gContext = new HttpContext();

    gRequestBodyBuffer = new Buffer();
    gResponseBuffer = new Buffer();

    gHttpProcessor->setBodyReaderCallBack(std::bind(&ProxyHandler::handleBody, &proxyHandler, _1, _2, _3));
    gHttpProcessor->setRequestHandlerCallBack(std::bind(&ProxyHandler::handleRequest, &proxyHandler, _1, _2));
    gContext->getResponse().setSendCallback(std::bind(&ProxyConnection::sendString, &proxyConnection, _1));
  
  } 

  virtual void tear_down()
  {
    delete gHttpProcessor;
    delete gContext;
    delete gRequestBodyBuffer;
    delete gResponseBuffer;
  }  

  void test_post()
  {
    HttpProcessor& httpProcessor = *gHttpProcessor;
    HttpContext& context = *gContext;
  
    Buffer buffer(0, 1024);
    buffer.append("POST / HTTP/1.1\r\n");
    assert(httpProcessor.process(buffer, context));
    assert(context.getState() == HttpContext::kRequestLineProcessed);

    buffer.append("Transfer-Encoding: chunked\r\n");
    buffer.append("\r\n");
    //cout << "state= "<<context.getState() << endl;
    assert(httpProcessor.process(buffer, context));
    
    assert(context.getState() == HttpContext::kPartChunkSizeSending);
    string encoding = context.getRequest().getHeader("Transfer-Encoding");
    assert(encoding == "chunked");


    buffer.append("3");
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kPartChunkSizeSending);

    buffer.append("\r\n12");  //got size
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kPartChunkBodySending);

    buffer.append("3\r\n");  //got body
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kPartChunkSizeSending);

    buffer.append("a\r");  //
    assert(httpProcessor.process(buffer, context));
    assert(context.getState() == HttpContext::kPartChunkSizeSending);

    buffer.append("\n");   //got size
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kPartChunkBodySending);

    buffer.append("0123456789\r\n"); //got body
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kPartChunkSizeSending);

    buffer.append("0\r\n\r\n");
    httpProcessor.process(buffer, context);
    assert(context.getState() == HttpContext::kAllChunkSent);

    cout << "request body start..." << endl;
    cout << gRequestBodyBuffer->toString() << endl;
    cout << "request body end..." << endl;

    assert ("1230123456789" == (gRequestBodyBuffer->toString()));

    cout << "response start..." << endl;
    cout << gResponseBuffer->toString() << endl;
    cout << "response end..." << endl;

    const char* chunckedPtr = gResponseBuffer->find("Transfer-Encoding: chunked\r\n");
    assert( NULL != chunckedPtr);

    const char* bodyPtr = gResponseBuffer->find("\r\n\r\n");
    bodyPtr += 4;
    std::string body = std::string(bodyPtr, gResponseBuffer->beginWrite() - bodyPtr);

    cout << "response body = " << body ;

    assert (body == "6\r\n123456\r\nc\r\n012345678901\r\n14\r\n01234567890123456789\r\n0\r\n\r\n");
  }

};
 
int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  TestChunkedHttpProcessor th;

  th.setup();
  th.test_post();
  th.tear_down();

}