#include <assert.h>
#include <vector>
#include "buffer.h"
#include "logger.h"

namespace libnet
{

size_t Buffer::append(const char* str, size_t len)
{
  ensureWritable(len);
  std::copy(str, str + len, beginWrite());
  moveWriteIndex(len);
  return len;
};

size_t Buffer::append(const Buffer &buffer)
{
  size_t len = buffer.readable();
  ensureWritable(len);
  std::copy(buffer.beginRead(), buffer.beginWrite(), beginWrite());
  moveWriteIndex(len);
  return len;
};

void Buffer::prepare(const char* str, size_t len)
{
  assert(prependable() > len);
  readIndex_ -= len;
  std::copy(str, str + len, cur());
};

void Buffer::prepareInt32(int32_t host32)
{
  size_t len = sizeof(int32_t);
  assert(prependable() > len);
  int32_t net32 = sockets::hostToNetwork32(host32);
  readIndex_ -= len;
  char* start = reinterpret_cast<char*>(&net32);
  std::copy(start, start + len, cur());
};

int32_t Buffer::peekInt32()
{
  size_t len = sizeof(int32_t);
  assert(readable() >= len);
  int32_t net32;
  ::memcpy(&net32, cur(), len);
  int32_t host32 = sockets::network32ToHost(net32);
  return host32;
};

bool Buffer::equals(const char* str)
{
  if (strlen(str) != readable()) return false;
  const char *s1 = str;
  const char *s2 = cur();

  while (*s1 != '\0')
  {
    if (*s1++ == *s2++) continue;
    else return false;
  }
  return true;
};

const char* Buffer::find(size_t pos, const char* str)
{
  assert (pos <= readable());
  for (size_t i = pos; i < readable(); i++)
  {
    bool found = true;
    for (size_t start = 0; start < strlen(str); start++)
    {
      if (start + i >= readable()) return NULL;
      if (at(start + i) == str[start]) continue;
      else 
      {
        found = false;
        break;
      }
    }
    if (found) return cur() + i;
  }
  return NULL;
};



const char* Buffer::rfind(const char* end, char ch)
{
  const char* start = beginRead();
  while (end >= start)
  {
    if (ch == (*end)) return end;
    end--;
  }
  return NULL;
};

//const char* Buffer::find()

void Buffer::makeRoom(size_t len)
{
  if (writable() + prependable() < len + prepend_)
    data_.resize(writeIndex_ + len);

  else
    compact();
};

void Buffer::compact()
{
  if (readIndex_ <= prepend_) return ;
  if (readIndex_ == writeIndex_)
  {
    readIndex_ = writeIndex_ = prepend_;
    return;
  }
  std::vector<char>::iterator start = data_.begin() + readIndex_; 
  std::vector<char>::iterator end = data_.begin() + writeIndex_;
  int distance = writeIndex_ - readIndex_;
  std::copy(start, end , data_.begin() + prepend_); 
  readIndex_ = prepend_;
  writeIndex_ = readIndex_ + distance;
};

std::string Buffer::toAsciiString()
{
  std::string str;
  str.reserve(1024);
  auto func = [&str](char ch)
  { str.push_back('[');
    str.append(std::to_string(static_cast<size_t>(ch)));
    str.push_back(']');
  };
  std::for_each(beginRead(), beginWrite(), func);
  return str;
};


}