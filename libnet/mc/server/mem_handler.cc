#include <libnet/timestamp.h>
#include <libnet/mc/mem_handler.h>
#include <libnet/mc/mcode.h>
#include <libnet/mc/message.h>
#include <libnet/mc/item.h>

namespace mc
{
namespace server
{
using namespace mc::msg;
static const size_t kSecondsOneMonth = 30 * 24 * 3600;

void MemHandler::handle(Message& request, Message& response)
{
  size_t hashcode = hash_func_(request.data_.key_);
  size_t shard = hashcode%shards_;
  LockGuard guard(locks_[shard]);
  MemCache& cache = *(caches_[shard]);
  if (request.data_.op_ == Opt::kGet)
  {
    Item* item = cache.find(request.data_.key_.c_str(), true);
    if (NULL ==  item)
    {
      response.stat_.code_ = kFail;
      return;
    }
    response.stat_.code_ = kSucc;
    response.data_.value_ = string(item->value());
    response.data_.flags_ = item->get_flags();
  }
  else if (request.data_.op_ == Opt::kSet || request.data_.op_ == Opt::kReplace || request.data_.op_ == Opt::kAdd)
  {
    Item* item = cache.find(request.data_.key_.c_str(), false);
    if (NULL == item)
    {
      if (request.data_.op_ == Opt::kReplace)
      {
        response.stat_.code_ = kFail;
        return;
      }
    }
    else
    {
      if (request.data_.op_ == Opt::kAdd)
      {
        response.stat_.code_ = kFail;
        return;
      }
    }
    if (NULL != item) cache.remove(item);
    Item* nitem = cache.alloc(request.data_.key_.size() + request.data_.value_.size() + 2);
    if (NULL == nitem)
    {
      LOG_ERROR << "can not alloc!" ;
      response.stat_.code_ = kError;
    }
    nitem->set_flags(request.data_.flags_);
    uint64_t now = Timestamp::now().secondsValue();
    if (request.data_.exptime_ == 0) nitem->set_exptime(0);
    else if (request.data_.exptime_ <= kSecondsOneMonth) nitem->set_exptime(now + request.data_.exptime_);// todo overflow
    else nitem->set_exptime(request.data_.exptime_ );
    nitem->set_time(now);
    string& key = request.data_.key_;
    nitem->set_key(key.c_str(), key.size());
    nitem->set_value(request.data_.value_.c_str(), request.data_.value_.size());
    nitem->set_hashcode(hash_func_(key));
    cache.add(nitem);
    response.stat_.code_ = kSucc;
    return;
  }
  else if (request.data_.op_ == Opt::kDelete)
  {
    Item* item = cache.find(request.data_.key_.c_str(), false);
    if (NULL != item)
    {
      response.stat_.code_ = kFail;
      return;
    }
    else
    {
      cache.remove(item);
      response.stat_.code_ = kSucc;
      return;
    }
  }
  else if (request.data_.op_ == Opt::kIncr || request.data_.op_ == Opt::kDecr)
  {
    Item* item = cache.find(request.data_.key_.c_str(), true);
    if (NULL != item)
    {
      response.stat_.code_ = kFail;
      return;
    }
    uint32_t value = 0;
    if (!digits::convert<uint32_t>(item->value(), value, 10))
    {
      LOG_ERROR << "key = " << request.data_.key_ << " error = convert " << (item->value()) << " to uint32_t fail!"; 
      response.stat_.code_ = kError;
      return;
    }
    if (request.data_.op_ == Opt::kIncr) //todo check overflow
      value += request.data_.count_;
    else
      value = (value <= request.data_.count_ ? 0 : value - request.data_.count_) ;
  
    std::string value_str = std::to_string(value);
    item->set_value(value_str.c_str(), value_str.size()); //todo should keep the minmum item can save a uint32_t number
    response.data_.count_ = value;
    return;
  }
  else if(request.data_.op_ == Opt::kVer)
  {
    response.stat_.code_ = kSucc;
    response.data_.value_ = "V1.0";
    return;
  }
  else
  {
    LOG_ERROR << "UNKNOWN op '" << request.data_.op_ << "'";
  }
}

}
}