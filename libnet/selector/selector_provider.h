#ifndef __LIBNET_SELECTOR_PROVIDER_H__
#define __LIBNET_SELECTOR_PROVIDER_H__

namespace libnet
{
class EventLoop;
namespace selector
{
class Selector;

class SelectorProvider
{
public:
    static Selector* provide(EventLoop *loop);
};

}
}

#endif