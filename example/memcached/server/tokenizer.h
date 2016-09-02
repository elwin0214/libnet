#ifndef __LIBNET_MEMCACHED_SERVER_TOKENIZER_H__
#define __LIBNET_MEMCACHED_SERVER_TOKENIZER_H__

#include <sys/types.h>
#include <vector>
#include <algorithm>
#include <libnet/nocopyable.h>

namespace memcached
{
namespace server
{

class Tokenizer
{
public:
  Tokenizer(const char* start, const char* end, char ch);

  bool next(const char* &pos, size_t &len);

private:
  const char* start_;
  const char* end_;
  const char ch_;
};

}
}
#endif