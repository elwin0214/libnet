#ifndef __LIBNET_SOCKET_DEF_H__
#define __LIBNET_SOCKET_DEF_H__
//#include "connection.h"

namespace libnet
{
  class Connection;
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void(const ConnectionPtr&)> ConnectionCallBack;
  typedef std::weak_ptr<Connection> WeakConnectionPtr;
}

#endif