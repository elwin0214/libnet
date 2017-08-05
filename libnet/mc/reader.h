#ifndef __LIBNET_MC_MSG_READER_H__
#define __LIBNET_MC_MSG_READER_H__
#include <string>
#include <algorithm>

namespace mc
{
namespace msg
{

class Reader
{
public:
  Reader(const char* start, const char* end, char sep)
    : start_(start),
      end_(end),
      sep_(sep)
  {

  }

  bool read(std::string& s);

private:
  bool next(const char* &pos, size_t &len);

private:
  const char* start_;
  const char* end_;
  const char sep_;
};

}
}
#endif