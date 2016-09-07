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

  void sendString(const CString& cstring)
  {
    gResponseBuffer->append(cstring.data(), cstring.length());
  }

};

ProxyHandler proxyHandler;
ProxyConnection proxyConnection;

struct TestHttpProcessorGet
{

  void setup()
  {
    gHttpProcessor = new HttpProcessor();
    gContext = new HttpContext();

    gRequestBodyBuffer = new Buffer();
    gResponseBuffer = new Buffer();

    gHttpProcessor->setBodyReaderCallBack(std::bind(&ProxyHandler::handleBody, &proxyHandler, _1, _2, _3));
    gHttpProcessor->setRequestHandlerCallBack(std::bind(&ProxyHandler::handleRequest, &proxyHandler, _1, _2));
    gContext->getResponse().setSendCallback(std::bind(&ProxyConnection::sendString, &proxyConnection, _1));  
  } 

  void tear_down()
  {
    delete gHttpProcessor;
    delete gContext;
    delete gRequestBodyBuffer;
    delete gResponseBuffer;
  }  

  void test_get_close()
  {
    gClose = true;
    Buffer buffer(1024);
    buffer.append("GET / HTTP/1.1\r\n\r\n");
    gContext->getResponse().setClose(true);
    gHttpProcessor->process(buffer, *gContext);
    assert(gContext->getState() == HttpContext::kBodySent);

    std::string response ="HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello world";
    cout << "http response start..." << endl;
    cout << (gResponseBuffer->toString());
    cout << "\nhttp response end..." << endl;
    assert (response == (gResponseBuffer->toString()));
  }

  void test_get_keepalive()
  {
    gClose = true;
    Buffer buffer(1024);
    buffer.append("GET / HTTP/1.1\r\n\r\n");
    gContext->getResponse().setClose(false);
    gHttpProcessor->process(buffer, *gContext);
    assert(gContext->getState() == HttpContext::kBodySent);

    std::string response ="HTTP/1.1 200 OK\r\nContent-Length: 11\r\nConnection: Keep-Alive\r\n\r\nhello world";
    cout << "http response start..." << endl;
    cout << (gResponseBuffer->toString());
    cout << "\nhttp response end..." << endl;
    assert (response == (gResponseBuffer->toString()));
  }

};
 
int main()
{
  TestHttpProcessorGet th;
  th.setup();
  th.test_get_close();
  th.tear_down();

  th.setup();
  th.test_get_keepalive();
  th.tear_down();
}