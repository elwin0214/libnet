#ifndef __LIBNET_NOCOPYABLE_H__
#define __LIBNET_NOCOPYABLE_H__

namespace libnet
{

class NoCopyable
{

protected:
  NoCopyable() {}
  ~NoCopyable() {}

private:
  NoCopyable(const NoCopyable &);
  const NoCopyable& operator=(const NoCopyable &);

};

}
#endif