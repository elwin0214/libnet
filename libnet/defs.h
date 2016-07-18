#ifndef __LIBNET_SOCKET_DEF_H__
#define __LIBNET_SOCKET_DEF_H__


namespace libnet
{
namespace socket
{
  
  class Connection;
  typedef std::function<void(ConnectionPtr)> ConnectionCallBack;

}
}

#endif