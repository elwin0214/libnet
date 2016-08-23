#include <iostream>
#include <assert.h>
#include "../atomic.h"

using namespace std;
using namespace libnet;
using namespace libnet::atomic;

struct TestAtomic

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

};
 
int main()
{
  TestAtomic ta;
  ta.test_cas_bool();
  ta.test_cas_int();
  return 0;
}

