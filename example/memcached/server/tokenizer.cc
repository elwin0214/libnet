#include "tokenizer.h"

namespace memcached
{
namespace server
{

struct CharEqual
{
  CharEqual(char ch)
    :ch_(ch)
  {

  };

  bool operator()(char ch)
  {
    return ch_ == ch;
  };

  char ch_;
};

struct CharNotEqual
{
  CharNotEqual(char ch)
    :ch_(ch)
  {

  };

  bool operator()(char ch)
  {
    return ch_ != ch;
  };

  char ch_;
};

Tokenizer::Tokenizer(const char* start, const char* end, char ch)
  : start_(start),
    end_(end),
    ch_(ch)
{

};

bool Tokenizer::next(const char* &pos, size_t &len)
{
  const char* start = std::find_if(start_, end_, CharNotEqual(ch_));
  if (start == end_) return false;
  const char* end = std::find_if(start, end_, CharEqual(ch_));
  pos = start;
  len = (end - start);
  start_ = end;
  return true;
};

}
}

