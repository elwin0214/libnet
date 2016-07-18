#include <functional>
#include <algorithm> 
#include <string>
#include <libnet/logger.h>
#include <libnet/buffer.h>
#include <libnet/http/http_request.h>
#include <libnet/digits.h>
#include <libnet/http/http_processor.h>
#include <iostream>

using namespace std::placeholders;

namespace libnet
{
namespace http
{

static const char* CRLF ="\r\n";
HttpProcessor::HttpProcessor()
    : defaultHandler_(),
      bodyReaderCallBack_(std::bind(&DefaultHandler::handleBody, &defaultHandler_, _1, _2, _3)),
      requestHandlerCallBack_(std::bind(&DefaultHandler::handleRequest, &defaultHandler_, _1, _2))
{

};

//
// @return true - succ , false - error
bool HttpProcessor::parseRequestLine(const char* begin, const char* end, HttpContext& context)
{ 
  const char* start = begin;
  const char* space = std::find(begin, end, ' ');
  HttpRequest& request = context.getRequest();

  if (!context.getRequest().setMethod(begin, space)) return false;

  start = space + 1;
  space = std::find(start, end, ' ');

  request.setPath(start, space);

  start = space + 1;

  if (end - start !=8) return false;
  if (!std::equal(start, end - 1 , "HTTP/1.")) return false;

  if ('0' == *(end - 1))
  {
    request.setVersion(HttpRequest::kHttp10);
  }
  else if ('1' == *(end - 1))
  {
    request.setVersion(HttpRequest::kHttp11);
  }
  else
  {
    return false;
  }
  return true;
};


bool HttpProcessor::parseRequestHeader(const char* begin, const char* end, HttpContext &context)
{ 
  const char* split = std::find(begin, end, ':');
  if (NULL == split)
    return false;
  HttpRequest& request = context.getRequest();
  request.addHeader(begin, split, end);
  return true;
};

bool HttpProcessor::processHeaders(HttpContext &context)
{
  HttpRequest& request = context.getRequest();
  std::string connection = request.getHeader("Connection");
  bool close = (connection == "close" || 
               (request.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive"));
  if (close)
  {
    context.getResponse().setClose(true);
  }
  if (request.getMethod() == HttpRequest::kGet)
  {
    return true;
  }
  std::string length = request.getHeader("Content-Length");
  //LOG_INFO << "length=" << length  <<".";

  if (length.size() > 0)
  {
    int len;
    digits::stringToDigit(length.c_str(), &len);  //fix error
    request.body.chunked_ = false;
    request.body.size_ = len;
    return true;
  }
  
  std::string transferEncoding = request.getHeader("Transfer-Encoding");
  //std::cout << (transferEncoding) << (transferEncoding.size())<< "|" << std::endl;
  //std::cout << ("chunked" == transferEncoding) << std::endl;
  //std::cout << (transferEncoding == "chunked") << std::endl;
  //LOG_INFO << "transferEncoding=" << transferEncoding  <<" equal=" << ("chunked" == transferEncoding) <<".";
  if ("chunked" == transferEncoding)
  {
    request.body.chunked_ = true;
    return true;
  }
  LOG_ERROR << "parse error!";
  return false;
};

size_t HttpProcessor::processBody(Buffer &input, size_t size, HttpContext &context)
{
  if (bodyReaderCallBack_)
  {
    return bodyReaderCallBack_(input, size, context);
  }
  return 0;
};

bool HttpProcessor::parseRequest(Buffer &input, HttpContext &context)
{
  HttpRequest& request = context.getRequest();
 
  const char* pos = NULL;
  bool done = false;
  bool result = false; // false - error , true - nedd more data
  while (!done)
  { 
    HttpContext::State state = context.getState();
    switch(state){

      case HttpContext::kInit :
      {
        pos = input.find(CRLF);
        if (NULL == pos)
        {
          done = true;
          result = true;
          break;
        }
        if (!parseRequestLine(input.beginRead(), pos, context))
        {
          done = true;
          result = false;
          break;
        }
        input.moveReadIndex(pos + 2 - input.beginRead());
        context.setState(HttpContext::kRequestLineReceived);
      }
        break;
      case HttpContext::kRequestLineReceived :
        context.setState(HttpContext::kRequestLineProcessed);
        break;

      //case HttpContext::kRequestLineProcessed : 
      case HttpContext::kRequestLineProcessed :
      {
        if (input.readable() <= 0) return true;
         
        pos = input.find(CRLF);
        if (NULL == pos)
        {
          done = true;
          result = true;
          break;
        } 
        if (pos == input.beginRead()) //\r\n
        {
          input.moveReadIndex(pos - input.beginRead() + 2);
          context.setState(HttpContext::kHeadersReceived);
          break;
        }
        if (!parseRequestHeader(input.beginRead(), pos, context))
        {
          done = true;
          result = false;
          break;
        }
        input.moveReadIndex(pos + 2 - input.beginRead());// got a header
      }
        break;

      case HttpContext::kHeadersReceived :
      { 
        if (!processHeaders(context))
        {
          done = true;
          result = false;
          break;
        }
        context.setState(HttpContext::kHeadersProcessed);
      }
        //return true; // callback outside
        break;
      case HttpContext::kHeadersProcessed :
        {
          if (headerHandlerCallBack_) 
            headerHandlerCallBack_(input, context);

          if (context.getRequest().getMethod() == HttpRequest::kGet)
          {
            context.setState(HttpContext::kBodySent); // finish
            requestHandlerCallBack_(input, context);
            done = true;
            result = true;
            break;
          }
          else
          {
            if (request.body.chunked_)
            {
              context.setState(HttpContext::kPartChunkSending);
            }
            else
            {
              context.setState(HttpContext::kBodySending);
            }
          }
        }
        break;

      case HttpContext::kBodySending :
      {
        if (input.readable() <= 0) 
        {
          done = true;
          result = true;
          break;
        }

        size_t allowed = std::min(input.readable(), request.body.remain());
        int used = processBody(input, allowed, context);
        if (used == 0) 
        {
          done = true;
          result = true;
          break;//return true; //need more
        }
        input.moveReadIndex(used);
        request.body.offset_ += used;
        if (request.body.remain() == 0)
        {
          context.setState(HttpContext::kBodySent);
          requestHandlerCallBack_(input, context);
          done = true;
          result = true;
          break;//return true; //need more
        }
      }
        break;
      case HttpContext::kPartChunkSending:
        context.setState(HttpContext::kPartChunkSizeSending);
      case HttpContext::kPartChunkSizeSending:
      {
        pos = input.find(CRLF);
        if (pos == NULL) 
        {
          done = true;
          result = true;
          break;//return true; //need more
        }
        const char* start = input.beginRead();
        std::string length = std::string(start, pos - start);
        int len;
        digits::xstringToDigit(length.c_str(), &(len));
        request.body.size_ = len;
        input.moveReadIndex(pos + 2 - input.beginRead());
        // if (request.body.size_ == 0)
        // { 
        //   if (input.readable() < 2)
        //     return true;
          
        //   input.moveReadIndex(2); //0\r\n
        //   context.setState(HttpContext::kAllChunkSent);
        //   requestHandlerCallBack_(input, context);
        //   return true;
        // }
        context.setState(HttpContext::kPartChunkSizeSent);
      }
        break;
      case HttpContext::kPartChunkSizeSent:
        context.setState(HttpContext::kPartChunkBodySending);

      case HttpContext::kPartChunkBodySending:
        {
          if (request.body.size_ == 0)
          { 
            if (input.readable() < 2)
            {
              done = true;
              result = true;
              break;//return true; //need more
            }
            input.moveReadIndex(2); //0\r\n
            context.setState(HttpContext::kAllChunkSent);
            requestHandlerCallBack_(input, context);

            done = true;
            result = true;
            break;//return true; //need more
          }
          //while (input.readable() > 0)
          //{
            if (request.body.remain() == 0)
            {
              if (input.readable() < 2)
              {
                done = true;
                result = true;
                break;//return true; //need more
              } 

              input.moveReadIndex(2);
              context.setState(HttpContext::kPartChunkSent);
              request.body.reset();
              //done = false;
              //result = true;
              break;//return true; //need more
            }
            size_t allowed = std::min(input.readable(), request.body.remain());
            int used = processBody(input, allowed, context);
            if (used == 0) 
            {
              done = true;
              result = true;
              break;//return true; //need more
            } 
            input.moveReadIndex(used);
            request.body.offset_ += used;
          
          //}
        }
        break;//return true;
      case HttpContext::kPartChunkSent:
        context.setState(HttpContext::kPartChunkSending);
        break;
      default:
        break;
    }//end switch
    //return false;
  }
  return result;

};

bool HttpProcessor::process(Buffer &input, HttpContext &context)
{
  return parseRequest(input, context);
};


}
}
