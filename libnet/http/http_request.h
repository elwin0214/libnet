#ifndef __LIBNET_HTTP_HTTPREQUEST_H__
#define __LIBNET_HTTP_HTTPREQUEST_H__

#include <map>
#include <string>
#include <libnet/http/http_bodybuffer.h>
#include <libnet/nocopyable.h>
#include <libnet/logger.h>

namespace libnet
{
class Connection;
namespace http
{

class HttpRequest : public NoCopyable
{

public:
  typedef std::map<std::string, std::string> Headers ;
  
  enum Method
  {
    kGet,
    kPost
  };

  enum Version
  {
    kHttp10,
    kHttp11
  };


public:
  HttpRequest()
    : path_(),
      method_(),
      version_(),
      headers_()
  {

  }

  void addHeader(const char* start, const char * split, const char* end) 
  {
    std::string key(start, split - start);
    std::string value(split + 2, end - split -2);
    LOG_TRACE << "key=" << key << " value=" << value ;
    headers_.insert(std::pair<std::string,std::string>(key, value));
    std::string v = getHeader("Transfer-Encoding");
  }

  std::string getHeader(const std::string &header)
  {
    std::string value;
    Headers::iterator itr = headers_.find(header);
    if (itr == headers_.end()) return value;
    else return itr->second;
  }

  void setPath(const char* start, const char* end)
  { 
    path_ = std::string(start, end - start);
  }

  const std::string& getPath() const { return path_; }

  void setVersion(HttpRequest::Version version) { version_ = version; }

  HttpRequest::Version getVersion() {return version_;}

  Method getMethod() { return method_; }

  void setMethod(HttpRequest::Method method) { method_ = method; }

  void reset()
  {
    std::string path;
    path_.swap(path);
    Headers headers;
    headers_.swap(headers);
    body.reset();
  }

  bool setMethod(const char* start, const char* end) 
  { 
    std::string str = std::string(start, end - start);
    if ("GET" == str)
    {
      method_ = kGet;
      return true;
    }
    else if ("POST" == str)
    {
      method_ = kPost;
      return true;
    }
    return false;
  }

private:
  friend class HttpProcessor;
  std::string path_;
  HttpRequest::Method method_;
  HttpRequest::Version version_;
  Headers headers_;
  BodyBuffer body;

};

}
}

#endif