#include "../tokenizer.h"
#include <libnet/buffer.h>
#include <string>
#include <assert.h>

using namespace std;
using namespace libnet;
using namespace memcached::server;


void test_incr()
{
  Buffer buffer;
  buffer.append("  incr key 1\r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = tokenizer.next(pos, len);
  if (next)
  {
    assert(std::string(pos, len) == "incr");
  }
};

void test_no_next()
{
  Buffer buffer;
  buffer.append(" \r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = tokenizer.next(pos, len);
  assert(!next);

};

void test_set()
{
  Buffer buffer;
  buffer.append(" set a 0 0 1\r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = false;
  next = tokenizer.next(pos, len);
  assert(std::string(pos, len) == "set");

  next = tokenizer.next(pos, len);
  assert(std::string(pos, len) == "a");

  next = tokenizer.next(pos, len);
  assert(std::string(pos, len) == "0");
};

int main()
{
  test_incr();
  test_no_next();
  test_set();
  return 0;
}