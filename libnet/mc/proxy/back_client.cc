#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/inet_address.h>
#include <libnet/mc/session.h>
#include "back_client.h"

namespace mc
{
namespace proxy
{

BackClient::BackClient(EventLoopGroup* loop_group, 
                       const std::vector<InetAddress>& remote_addresses,
                       CountDownLatch& connected_latch,
                       CountDownLatch& closed_latch)
  : sessions_(),
    size_(remote_addresses.size()),
    hash_(std::hash<string>())
{
  sessions_.reserve(size_);
  for (int i = 0; i < size_; i++)
  {
    auto s = make_shared<Session>(loop_group->getNextLoop(), 
                                  remote_addresses[i], 
                                  connected_latch, 
                                  closed_latch, 1);
    sessions_.push_back(std::move(s));
  }
}

void BackClient::connect()
{
  for (auto& session : sessions_)
    session->connect();
}

void BackClient::enableRetry()
{
  for (auto& session : sessions_)
    session->enableRetry();
}

void BackClient::disconnect()
{
  for (auto& session : sessions_)
  {
    session->disconnect();
  }
}

void BackClient::send(const Cmd& cmd)
{
  size_t h = hash_(cmd->request().data_.key_);
  sessions_[h % size_]->send(cmd, 0, false);
}

}
}