#include <iostream>
#include <vector>
#include <cpptest.h>
#include <libnet/logger.h>
#include <libnet/buffer.h>
#include <assert.h>

using namespace std;
using namespace libnet;

struct TestBuffer  : public Test::Suite
{
  void test_append()
  {
    Buffer buf(0, 10);
    buf.append("abc", 3);
    TEST_ASSERT(string(buf.cur(), buf.readable()) == "abc");
    assert(string(buf.cur(), buf.readable()) == "abc");
  }

  void test_appendOver()
  {
    Buffer buf(0, 2);
    buf.append("abc", 3);
    TEST_ASSERT(string(buf.cur(), buf.readable()) == "abc");
    assert((string(buf.cur(), buf.readable()) == "abc"));

    TEST_ASSERT(buf.capcity() >= 3);
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
    TEST_ASSERT(string(buf.cur(), buf.readable()) == "c");
    assert((string(buf.cur(), buf.readable()) == "c"));

    buf.append("de", 2);
    TEST_ASSERT(string(buf.cur(), buf.readable()) == "cde");
    assert(string(buf.cur(), buf.readable()) == "cde");

    TEST_ASSERT((buf.readIndex()) == 0);
    assert((buf.readIndex()) == 0);
  }

  void test_equal()
  {
    Buffer buf(0, 2);
    buf.append("exit\r\n", 6);
     
    TEST_ASSERT(buf.equals("exit\r\n"));
    TEST_ASSERT(!buf.equals("abcd"));
    TEST_ASSERT(!buf.equals("ab"));

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
    TEST_ASSERT((*p == '\r'));
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
    TEST_ADD(TestBuffer::test_append);
    TEST_ADD(TestBuffer::test_appendOver);
    TEST_ADD(TestBuffer::test_move);
    TEST_ADD(TestBuffer::test_equal);
    TEST_ADD(TestBuffer::test_find);
    TEST_ADD(TestBuffer::test_prepre);
    TEST_ADD(TestBuffer::test_startWiths);
  }
};
 
int main()
{
  TestBuffer tb;
  Test::TextOutput output(Test::TextOutput::Verbose);
  (tb.run(output, false));
}

