#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include <random>
#include <memory>
#include <iostream>
#include <libnet/mc/command.h>
#include <libnet/mc/async_client.h>
#include <libnet/mc/req_cache.h>
#include <libnet/mc/session.h>

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;

AsyncClient::AsyncClient(EventLoop* loop, 
                         const char* host, 
                         uint16_t port, 
                         CountDownLatch& connected_latch,
                         CountDownLatch& closed_latch,
                         size_t conn_size,
                         int32_t send_wait_milli,
                         size_t high_water_mark,
                         uint32_t idle_timeout_milli,
                         size_t max_retry)
  : send_wait_milli_(send_wait_milli)
{
  sessions.reserve(conn_size);
  for (size_t i = 0; i < conn_size; i++)
  {
    auto s = make_shared<Session>(loop,
                                  host,
                                  port,
                                  connected_latch,
                                  closed_latch,
                                  high_water_mark,
                                  idle_timeout_milli,
                                  max_retry);
    sessions.push_back(std::move(s));
  }
};

void AsyncClient::connect()
{
  for (auto itr = sessions.begin(); itr != sessions.end(); itr++)
    (*itr)->connect();
};

void AsyncClient::disconnect()
{
  for (auto itr = sessions.begin(); itr != sessions.end(); itr++)
    (*itr)->disconnect();
};

void AsyncClient::enableRetry()
{
  for (auto itr = sessions.begin(); itr != sessions.end(); itr++)
    (*itr)->enableRetry();
};

future<AsyncClient::Msg> AsyncClient::sendFuture(Message request)
{
  auto p = make_shared<std::promise<AsyncClient::Msg>>() ;
  auto f = p->get_future(); 
  shared_ptr<Command> cmd = make_shared<Command>(request, [p](const AsyncClient::Msg& msg)mutable{ p->set_value(msg); });
  send(cmd);
  return f;//NRVO
};

void AsyncClient::send(const std::shared_ptr<Command>& command)
{ 
  int size = sessions.size();
  int index = random(0, size - 1);
  if (sessions[index]->connected())
  {
    if (!sessions[index]->send(command, send_wait_milli_))
    {
      command->response().stat_.code_ = kCacheReject;
      command->call();
    }
    return;
  }
  int last = index;
  int next = index + 1;
  next = next % size;
  for (; next != last; next++)
  {
    next = next % size;
    if (sessions[index]->connected())
    {
      if (!sessions[index]->send(command, send_wait_milli_))
      {
        command->response().stat_.code_ = kCacheReject;
        command->call();
      }
      return;
    }
  }
  command->response().stat_.code_ = kConClosed;
  command->call();
};

int AsyncClient::random(const int& min, const int& max) 
{
  static thread_local std::mt19937 generator;
  std::uniform_int_distribution<int> distribution(min,max);
  return distribution(generator);
};

}
}
