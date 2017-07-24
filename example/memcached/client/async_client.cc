#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include <random>
#include <memory>
#include <iostream>
#include "command.h"
#include "async_client.h"

#include "cache.cc"
#include "client_impl.cc"

namespace memcached
{
namespace client
{
using namespace libnet;
using namespace std;

AsyncClient::AsyncClient(EventLoop* loop, const char* host, int port, CountDownLatch& latch, int connSize)
  : latch_(latch)
{
  impls.reserve(connSize);
  for (int i = 0; i < connSize; i++)
  {
    shared_ptr<ClientImpl> impl(new ClientImpl(loop, host, port, latch));
    impls.push_back(std::move(impl));
  }
};

void AsyncClient::connect()
{
  for (auto itr = impls.begin(); itr != impls.end(); itr++)
    (*itr)->connect();
};

void AsyncClient::disconnect()
{
  for (auto itr = impls.begin(); itr != impls.end(); itr++)
    (*itr)->disconnect();
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
  int size = impls.size();
  int index = random(0, size - 1);
  if (impls[index]->connected())
  {
    impls[index]->send(command);
    return;
  }
  int last = index;
  int next = index + 1;
  next = next % size;
  for (; next != last; next++)
  {
    next = next % size;
    if (impls[index]->connected())
    {
      impls[index]->send(command);
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
