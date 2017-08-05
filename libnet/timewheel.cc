#include <libnet/timewheel.h>

namespace libnet
{

void TimeWheel::onConnection(const Conn& conn)
{
  if (conn->connected())
  {
    EntryPtr entry(new Entry(conn));
    WeakEntryPtr wk_entry = entry;
    conn->setWeakContext(wk_entry);
    buckets_[index_].insert(std::move(entry));
  }
};

void TimeWheel::onMessage(const Conn& conn)
{
  std::weak_ptr<void> wk_ptr = conn->getWeakContext();
  std::shared_ptr<void> ptr = wk_ptr.lock();
  if (!ptr) 
    return;
  EntryPtr entry_ptr = std::static_pointer_cast<Entry>(ptr);
  buckets_[index_].insert(entry_ptr);
};

void TimeWheel::rotate()
{
  index_ = (index_ + 1) % buckets_.size();
  buckets_[index_].clear();
};

}