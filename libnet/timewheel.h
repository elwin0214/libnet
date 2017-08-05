#ifndef __LIBNET_TIMEWHEEL_H__
#define __LIBNET_TIMEWHEEL_H__

#include <memory>
#include <functional>
#include <unordered_set>
#include <libnet/connection.h>
#include <libnet/nocopyable.h>

namespace libnet
{
typedef std::shared_ptr<Connection> Conn;
typedef std::weak_ptr<Connection> WeakConn;

class Entry
{
public:
  Entry(const Conn& conn)
    : weak_conn_(conn)
  {

  }
  
  ~Entry()
  {
    Conn conn = weak_conn_.lock();
    if (conn)
    {
      conn->shutdown();
    }
  }
private:
  WeakConn weak_conn_;
};

class TimeWheel : public NoCopyable
{

public:
  typedef std::shared_ptr<Entry> EntryPtr;
  typedef std::weak_ptr<Entry> WeakEntryPtr;
  typedef std::unordered_set<EntryPtr> Bucket;

  TimeWheel(size_t size)
    : buckets_(size),
      index_(0)
  {

  }

  void onConnection(const Conn& conn);
  void onMessage(const Conn& conn);
  void rotate();

private:
  std::vector<Bucket> buckets_;
  size_t index_;
};

}
#endif


