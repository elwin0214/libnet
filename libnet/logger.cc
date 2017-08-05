#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <libnet/timestamp.h>
#include <libnet/logger.h>
#include <libnet/current_thread.h>

namespace libnet
{
namespace log
{

const char* LogLevelName[6] = 
{
  "TRACE",
  "DEBUG",
  "INFO ",
  "WARN ",
  "ERROR",
  "FATAL",
};


LoggerStream& LoggerStream::operator << (const std::string &str)
{
  if (closed() || buffer_.remain() <= 0) return *this;
  //The snprintf() and vsnprintf() functions will write at most n-1 of the characters printed into the
  //   output string (the n'th character then gets the terminating `\0');
  int size = snprintf(buffer_.cur(), buffer_.remain(), "%s", str.c_str());
  buffer_.move(size);
  return *this;
};

LoggerStream::~LoggerStream()
{

};

LoggerStream& LoggerStream::append(const char *format_string, ...)
{
  if (closed() || buffer_.remain() <= 0) return *this;
  va_list ap;
  va_start(ap, format_string);
  int size = vsnprintf(buffer_.cur(), buffer_.remain() , format_string, ap);
  va_end(ap);
  buffer_.move(size);//maybe bug
  return *this;
};

void LoggerStream::init()
{
  if (closed()|| buffer_.remain() <= 0) return ;
  Timestamp now = Timestamp::now();

  time_t seconds = static_cast<time_t>(now.value() / Timestamp::kMicroSecondsPerSecond);
  struct tm time;
  ::gmtime_r(&seconds, &time);
  int size = snprintf(buffer_.cur(),buffer_.remain(), "%4d%02d%02d %02d:%02d:%02d",
             time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
             time.tm_hour, time.tm_min, time.tm_sec);
  buffer_.move(size);
};

Logger::Impl::Impl(const char* file, const char* func, int line, int err, LogLevel level)
    : stream_(),
      file_(file),
      func_(func),
      line_(line),
      level_(level)
{
  if (level_ < Logger::getLogLevel())
  {
    stream_.close();
  }
  else
  {
    stream_.init();
    stream_.append(" %s ", LogLevelName[level_]);
    const char *filename = strrchr(file, '/'); 
    if (NULL != filename) 
      filename = filename + 1;
    else 
      filename = file;
    stream_.append("%s::%s", filename, func);
    stream_.append(" (%d) ", line);
    stream_.append("[%lx] ", thread::currentTid());
    stream_.append("[%s] ", thread::currentThreadName());
    if (err != 0)
      stream_ << Error() << "(errno=" << err << ") ";
  }
};

void Logger::Impl::finish()
{
  if (stream_.closed()) return;
  stream_ << "\n";
};

Logger::Logger(const char* file, const char* func, int line, LogLevel level)
    : impl_(file, func, line, 0, level)
{

};

Logger::Logger(const char* file, const char* func, int line, bool abort)
  : //abort_(abort),
    impl_(file, func, line, errno, abort ? FATAL : ERROR) 
    
{

};

Logger::~Logger()
{
  if (stream().closed()) return;
  impl_.finish();
  outputFunc_(stream().buffer().data(), stream().buffer().size());
  flushFunc_();
  if (impl_.level_ == FATAL)
  {
    flushFunc_();
    abort();
  }
}
void defaultOutput(const char* msg, int len)
{
  fwrite(msg, 1, len, stdout);
};

void defaultFlush()
{
  fflush(stdout);
}
LogLevel Logger::logLevel_ = INFO;
Logger::OutputFunc Logger::outputFunc_ = defaultOutput;
Logger::FlushFunc Logger::flushFunc_ = defaultFlush;
 

}
}