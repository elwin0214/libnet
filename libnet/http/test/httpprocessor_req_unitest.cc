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
    //response.setClose(gClose);
    response.send("hello world", 11);
    response.flush();
  }
};

//mock connection
struct ProxyConnection
{
  void sendBuffer(Buffer* buffer)
  {
    //cout << "sendBuffer=" << (buffer->toString()) << endl; 
    gResponseBuffer->append(buffer->beginRead(), buffer->readable());
    buffer->clear();
    delete buffer;
  }

  void sendString(CString cstring)
  {
    gResponseBuffer->append(cstring.data(), cstring.length());
  }

};

ProxyHandler proxyHandler;
ProxyConnection proxyConnection;



struct TestHttpProcessor 
{
  void setup()
  {
    gHttpProcessor = new HttpProcessor();
    gContext = new HttpContext();

    gRequestBodyBuffer = new Buffer();
    gResponseBuffer = new Buffer();

    gHttpProcessor->setBodyReaderCallBack(std::bind(&ProxyHandler::handleBody, &proxyHandler, _1, _2, _3));
    gHttpProcessor->setRequestHandlerCallBack(std::bind(&ProxyHandler::handleRequest, &proxyHandler, _1, _2));
    gContext->getResponse().setSendCallback(std::bind(&ProxyConnection::sendBuffer, &proxyConnection, _1));
    gContext->getResponse().setSendStringCallback(std::bind(&ProxyConnection::sendString, &proxyConnection, _1));
  
  } 

  void tear_down()
  {
    delete gHttpProcessor;
    delete gContext;
    delete gRequestBodyBuffer;
    delete gResponseBuffer;
  }  

  void test_post()
  {
 
 
    Buffer buffer(1024);
    buffer.append("POST / HTTP/1.1\r\n");
    buffer.append("Content-Length: 3\r\n");
    buffer.append("\r\n");
    assert(gHttpProcessor->process(buffer, *gContext));
    assert(gContext->getState() == HttpContext::kBodySending);
    buffer.append("12");

   
    gHttpProcessor->process(buffer, *gContext);
    assert(gContext->getState() == HttpContext::kBodySending);

    buffer.append("3");
    gHttpProcessor->process(buffer, *gContext);
    assert(gContext->getState() == HttpContext::kBodySent);

    assert ("123" == gRequestBodyBuffer->toString());//
    cout << "request body=" << gResponseBuffer->toString() << endl;
  }


};
 
int main()
{
  TestHttpProcessor th;
  th.setup();
  th.test_post();
  th.tear_down();
}