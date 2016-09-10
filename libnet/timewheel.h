#ifndef __LIBNET_TIMEWHEEL_H__
#define __LIBNET_TIMEWHEEL_H__

#include <memory>
#include <functional>
#include <unordered_set>
#include "defs.h"
#include "connection.h"
#include "nocopyable.h"

namespace libnet
{

class Entry
{
public:
  Entry(const ConnectionPtr& connection)
    : weak_connection_(connection)
  {

  }
  
  ~Entry()
  {
    ConnectionPtr connection = weak_connection_.lock();
    if (connection)
    {
      connection->shutdown();
    }
  }
private:
  WeakConnectionPtr weak_connection_;
};

class TimeWheel : public NoCopyable
{

public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::shared_ptr<Entry> EntryPtr;
  typedef std::weak_ptr<Entry> WeakEntryPtr;
  typedef std::unordered_set<EntryPtr> Bucket;

  TimeWheel(size_t size)
    : buckets_(size),
      index_(0)
  {

  }

  void onConnection(const ConnectionPtr& connection);
  void onMessage(const ConnectionPtr& connection);
  void rotate();

private:
  std::vector<Bucket> buckets_;
  size_t index_;
};

}
#endif


