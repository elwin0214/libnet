#include "timewheel.h"

namespace libnet
{

void TimeWheel::onConnection(const ConnectionPtr& connection)
{
  if (connection->connected())
  {
    EntryPtr entry(new Entry(connection));
    WeakEntryPtr wk_entry = entry;
    connection->setWeakContext(wk_entry);
    buckets_[index_].insert(std::move(entry));
  }
};

void TimeWheel::onMessage(const ConnectionPtr& connection)
{
  std::weak_ptr<void> wk_ptr = connection->getWeakContext();
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