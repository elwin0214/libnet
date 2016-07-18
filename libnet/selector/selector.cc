#include "selector.h"

namespace libnet
{
namespace selector
{

Selector::Selector(EventLoop *loop):loop_(loop),channels_()
{

};

Selector::~Selector()
{

};

}
}
