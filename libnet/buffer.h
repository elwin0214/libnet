#ifndef __LIBNET_BUFFER_H__
#define __LIBNET_BUFFER_H__

#include <sys/types.h>
#include <vector>
#include <string>
#include <assert.h>
#include "nocopyable.h"
#include "cstring.h"
#include "endian_ops.h"
#include "socket_ops.h"

namespace libnet
{

class Buffer : public NoCopyable 
{

static const size_t kPrepend = 8;
static const size_t kInitSize = 1024;
public:
  explicit Buffer(size_t prepend = kPrepend, size_t size = kInitSize)
    : prepend_(prepend),
      readIndex_(prepend),
      writeIndex_(prepend),
      data_(prepend + size)
  {

  }

  size_t readIndex() const {return readIndex_; }
  size_t writeIndex() const {return writeIndex_; }
  size_t readable() const {return writeIndex_ - readIndex_; }
  size_t writable() const {return data_.capacity() - writeIndex_; }
  size_t prependable() const {return readIndex_; } 

  char* cur() { return data_.data() + readIndex_; }
  const char* cur() const {return data_.data() + readIndex_; }

  char* beginRead() { return data_.data() +  readIndex_; }
  char* beginWrite() { return data_.data() + writeIndex_; }
  const char* beginRead() const { return data_.data() +  readIndex_; }
  const char* beginWrite() const { return data_.data() + writeIndex_; }

  void moveReadIndex(size_t len)
  {
    if (len <= readable())
      readIndex_ += len; 
    else
      clear();
  };

  void moveWriteIndex(size_t len)
  {
    if (len <= writable())
      writeIndex_ += len; 
    else
      writeIndex_ = data_.capacity();
  };
  
  size_t append(const char* str, size_t n);
  size_t append(const Buffer &buffer);
  size_t append(const CString& cstring, size_t from)
  {
    return append(cstring.data() + from, cstring.length());
  };

  size_t append(const std::string& str) 
  { 
    return append(str.data(), str.size()); 
  }
  
  void ensureWritable(size_t len)
  {
    if (writable() < len)
      makeRoom(len);
  }

  void clear() { readIndex_ = writeIndex_ = prepend_; }
  bool equals(const char* str);
  size_t capcity() { return data_.capacity(); }
 
  std::string toString()
  {
    return std::string(cur(), readable());
  };

  std::string toAsciiString()
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

  const char& at(size_t index)
  {
    assert(index < readable());
    return *(beginRead() + index);//data_[readIndex_ + index];
  };

  bool startWiths(const char* str)
  {
    return startWiths(str, ::strlen(str));
  };

  bool startWiths(const char* str, size_t len)
  {
    size_t size = readable();
    if (size < len) return false;
    size_t nlen = size < len ? size : len;
    return ::strncmp(beginRead(), str, nlen) == 0;
  };

  void makeRoom(size_t len);

  const char* find(const char* str)
  {
    return find(0, str);
  }

  const char* find(size_t pos, const char* str);
  const char* rfind(const char* end, char ch);

  void prepare(const char* str, size_t len);

  void prepareInt32(int32_t host32);

  int32_t peekInt32();

  int32_t readInt32()
  {
    int32_t n = peekInt32();
    moveReadIndex(sizeof(int32_t));
    return n;
  };

  void swap(Buffer &buffer)
  {
    std::swap(prepend_, buffer.prepend_);
    std::swap(readIndex_, buffer.readIndex_);
    std::swap(writeIndex_, buffer.writeIndex_);
    data_.swap(buffer.data_); 
  };

  void shrink(size_t len)
  {
    Buffer buffer;
    buffer.ensureWritable(this->readable() + len);
    buffer.append(*this);
    swap(buffer);
  };
private:
  void compact();
    
private:
  size_t prepend_;
  size_t readIndex_;
  size_t writeIndex_;
  std::vector<char> data_;
};

}

#endif