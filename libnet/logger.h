#ifndef __LIBNET_LOG_LOGGER_H__
#define __LIBNET_LOG_LOGGER_H__

#include <errno.h>
#include <string.h>
#include <string>
#include <libnet/nocopyable.h>

namespace libnet
{
namespace log
{

struct Error 
{
  Error():err_(errno)
  {

  }

  Error(int err):err_(err)
  {

  }
  
  int err_;
};

template<int SIZE>
class Buffer
{
public:
    
  Buffer():cur_(data_){bzero(data_, sizeof(data_)); }
  char* cur() { return cur_; }
  char* data() { return data_; }
  int remain() { return data_ + sizeof(data_) - cur_; }
  void move(int n) { cur_ = cur_ + n; } 
  void add(char c) { if (cur_ - data_ < (sizeof(data_))) *cur_++ = c; }
  //void close() {*cur_ = '\0';}
  size_t size() { return static_cast<size_t>(cur_ - data_); }
  std::string toString() {return std::string(data_, size());}
private:
  char data_[SIZE];
  char *cur_;

};

enum LogLevel
{
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
};

class LoggerStream : public NoCopyable 
{
friend class Logger;

public:
  typedef LoggerStream self;

  LoggerStream():buffer_(),closed_(false)
  {

  }

  ~LoggerStream();

  self& operator << (const std::string &str);
  self& operator << (int v) { append("%d", v); return *this; }
  self& operator << (unsigned int v) { append("%d", v); return *this; }
  self& operator << (long v) { append("%ld", v); return *this; }
  self& operator << (unsigned long v) { append("%ld", v); return *this; }
  self& operator << (long long v) { append("%lld", v); return *this; }
  self& operator << (unsigned long long v) { append("%lld", v); return *this; }

  self& operator << (double v) { append("%.2f", v); return *this; }
  //self& operator << (uint64_t v) {append("%lld", v); return *this; }
  self& operator << (void* p) { append("%x", p); return *this; }

  self& operator << (const char* str){ append("%s", str); return *this; }

  self& operator << (const Error& e) 
  {
    char buf[100];
    ::bzero(buf, sizeof(buf));
    #ifdef __APPLE__
      strerror_r(e.err_, buf, 100);
      return append("%s", buf);
    #else
      return append("%s", strerror_r(e.err_, buf, 100));// should use the returned pointer
    #endif
  }
  Buffer<1024>& buffer() {return buffer_; }
  void close() {closed_ = true; }
  bool closed() {return closed_; }
private:
  self& append(const char *format_string, ...);
  void init();
  

private:
  Buffer<1024> buffer_;
  bool closed_;
  //LogLevel logLevel_;
};

class Logger : public NoCopyable
{

public:
  Logger(const char* file, const char* func, int line, LogLevel level);
  Logger(const char* file, const char* func, int line, bool abort);
  LoggerStream& stream() {return impl_.stream_; }

  static void setLogLevel(LogLevel logLevel) {Logger::logLevel_ = logLevel; }
  static LogLevel getLogLevel() {return Logger::logLevel_; }

  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)(); 
 
  static void setOutputFunc(OutputFunc func) {Logger::outputFunc_ = func; } 
  static void setFlushFunc(FlushFunc func) {Logger::flushFunc_ = func; } 
  ~Logger();

private:
  static LogLevel logLevel_;
  static OutputFunc outputFunc_;
  static FlushFunc flushFunc_;

  struct Impl
  {
    Impl(const char* file, const char* func, int line, int err, LogLevel level);
    void finish();

    LoggerStream stream_;
    const char* file_;
    const char* func_;
    int line_;
    LogLevel level_;
    
  };
  Impl impl_;
};

}

inline void setLogLevel(log::LogLevel logLevel)
{
  log::Logger::setLogLevel(logLevel);
};

#define LOG_TRACE libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::TRACE).stream()
#define LOG_DEBUG libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::DEBUG).stream()
#define LOG_INFO libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::INFO).stream()  
#define LOG_WARN libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::WARN).stream()
#define LOG_ERROR libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::ERROR).stream()
#define LOG_FATAL libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__,libnet::log::FATAL).stream()

#define LOG_SYSERROR libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__, false).stream()
#define LOG_SYSFATAL libnet::log::Logger(__FILE__, __FUNCTION__, __LINE__, true).stream()


#define ERR_DESC libnet::log::Error()

}


#endif

