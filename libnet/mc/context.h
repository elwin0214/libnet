#ifndef __LIBNET_MC_SERVER_CONTEXT_H__
#define __LIBNET_MC_SERVER_CONTEXT_H__
#include <functional>
#include <string>
#include <libnet/nocopyable.h>
#include "message.h"

namespace mc
{
namespace server
{

class Context : public libnet::NoCopyable
{

public:
  Context()
  {

  };

  Message& request() { return request_; }
private:
  Message request_;

};

}
}

#endif