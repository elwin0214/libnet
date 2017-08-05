#include <libnet/mc/reader.h>

namespace mc
{
namespace msg
{

bool Reader::next(const char* &pos, size_t &len)
{
  const char* start = std::find_if(start_, end_, [this](char c){return c != this->sep_; });
  if (start == end_) return false;
  const char* end = std::find_if(start, end_, [this](char c){return c == this->sep_; });
  pos = start;
  len = (end - start);
  start_ = end;
  return true;
};

bool Reader::read(std::string& s)
{
  const char* p = NULL;
  size_t len = 0;
  if (!next(p, len)) return false;
  s = std::string(p, p + len);
  return true;
};

}
}