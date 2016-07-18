#ifndef __LIBNET_ATOMIC_BOOL_H__
#define __LIBNET_ATOMIC_BOOL_H__
#include "nocopyable.h"

namespace libnet
{

namespace atomic
{
class AtomicBool: public NoCopyable 
{

public:
  explicit AtomicBool(bool v):value_(v) 
  {

  }

  bool cas(bool expect, bool value)
  {
    return __sync_bool_compare_and_swap(&value_, expect, value);
  }

  void set(bool value)
  {
    value_ = value;
  }

  bool get()
  {
    return value_;
  }

private:
  volatile bool value_;

};

template<typename T>
class AtomicInteger : public NoCopyable 
{

public:
  explicit AtomicInteger(T v):value_(v) 
  {

  }

  bool cas(T expect, T value)
  {
    return __sync_bool_compare_and_swap(&value_, expect, value);
  }

  T getAndAdd(T n)
  {
    return __sync_fetch_and_add(&value_, n);
  }

  T addAndGet(T n)
  {
    return getAndAdd(n) + n;
  }

private:
  volatile T value_;

};
}

typedef atomic::AtomicInteger<int32_t> AtomicInt32;
typedef atomic::AtomicInteger<int64_t> AtomicInt64;
typedef atomic::AtomicBool AtomicBool;

}
#endif