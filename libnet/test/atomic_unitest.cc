#include <iostream>
#include <cpptest.h>
#include <assert.h>
#include "../atomic.h"

//#include <libnet/atomic.h>

using namespace std;
using namespace libnet;
using namespace libnet::atomic;

struct TestAtomic : public Test::Suite
{

  void test_cas_bool()
  {
    AtomicBool b(false);
    bool t = b.cas(false, true);
    assert(t);
    t = b.cas(false, true);
    assert(!t);

  }

  void test_cas_int()
  {
    AtomicInt32 atomicInt(0);
    for (int i = 0; i < 1024 ; i++)
    {
      assert((i + 1) == atomicInt.addAndGet(1));
     // TEST_ASSERT((i + 1) == atomicInt.addAndGet(1));
    }

  }

  TestAtomic()
  {
    TEST_ADD(TestAtomic::test_cas_bool);
    TEST_ADD(TestAtomic::test_cas_int);
  }

  ~TestAtomic()
  {
    
  }
};
 
int main()
{
  TestAtomic ta;
  Test::TextOutput output(Test::TextOutput::Verbose);
  (ta.run(output, false));
  return 0;
}

