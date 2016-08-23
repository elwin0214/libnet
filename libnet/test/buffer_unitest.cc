#include <iostream>
#include <vector>
#include <libnet/logger.h>
#include <libnet/buffer.h>
#include <assert.h>

using namespace std;
using namespace libnet;

struct TestBuffer
{
  void test_append()
  {
    Buffer buf(0, 10);
    buf.append("abc", 3);
    assert(string(buf.cur(), buf.readable()) == "abc");
  }

  void test_appendOver()
  {
    Buffer buf(0, 2);
    buf.append("abc", 3);
    assert((string(buf.cur(), buf.readable()) == "abc"));

    assert(buf.capcity() >= 3);
    vector<char> vec;
    vec.resize(2);
    vec.resize(3); //4
    //cout << "vec.capcity = " << vec.capacity() << endl;
  }

  void test_move()
  {
    Buffer buf(0, 2);
    buf.append("abc", 3);
    buf.moveReadIndex(2);
    assert((string(buf.cur(), buf.readable()) == "c"));

    buf.append("de", 2);
    assert(string(buf.cur(), buf.readable()) == "cde");

    assert((buf.readIndex()) == 0);
  }

  void test_equal()
  {
    Buffer buf(0, 2);
    buf.append("exit\r\n", 6);

    assert(buf.equals("exit\r\n"));
    assert(!buf.equals("abcd"));
    assert(!buf.equals("ab"));
  }
  
  void test_find()
  {
    Buffer buf(0, 100);
    const char *s = "GET /index HTTP/1.1\r\n";
    buf.append(s, strlen(s));
    const char *p = buf.find("\r\n");
    assert((*p == '\r'));
  }

  void test_prepre()
  {
    Buffer buf(16, 1024);
    buf.append("0123456789", 10);
    buf.prepare("abc", 3);
    assert(("abc0123456789" == buf.toString()));
  }


  void test_startWiths()
  {
    Buffer buf(16, 1024);
    buf.append("0123456789", 10);

    assert(buf.startWiths("0123", 4));
    assert(buf.startWiths("0123", 2));
    assert(!buf.startWiths("2123", 2));
  }


  TestBuffer()
  {

  }
};
 
int main()
{
  TestBuffer tb;
  tb.test_append();
  tb.test_appendOver();
  tb.test_move();
  tb.test_equal();
  tb.test_find();
  tb.test_prepre();
  tb.test_startWiths();

}

