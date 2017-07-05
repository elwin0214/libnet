#include <libnet/timestamp.h>
#include "memhandler.h"
#include "../message/mcode.h"
#include "../message/request.h"
#include "../message/response.h"

namespace memcached
{
namespace server
{
using namespace memcached::message;
static const size_t kSecondsOneMonth = 30 * 24 * 3600;

void MemHandler::handle(Request& request, Response& response)
{
  if (request.op_ == Opt::kGet)
  {
    Item* item = cache_.find(request.key_.c_str(), true);
    if (NULL ==  item)
    {
      response.code_ = kFail;
      return;
    }
    response.code_ = kSucc;
    response.value_ = string(item->value());
    response.flags_ = item->get_flags();
  }
  else if (request.op_ == Opt::kSet || request.op_ == Opt::kReplace || request.op_ == Opt::kAdd)
  {
    Item* item = cache_.find(request.key_.c_str(), false);
    if (NULL == item)
    {
      if (request.op_ == Opt::kReplace)
      {
        response.code_ = kFail;
        return;
      }
    }
    else
    {
      if (request.op_ == Opt::kAdd)
      {
        response.code_ = kFail;
        return;
      }
    }
    if (NULL != item) cache_.remove(item);
    Item* nitem = cache_.alloc(request.key_.size() + request.value_.size() + 2);
    if (NULL == nitem)
    {
      LOG_ERROR << "can not alloc!" ;
      response.code_ = kError;
    }
    nitem->set_flags(request.flags_);
    uint64_t now = Timestamp::now().secondsValue();
    if (request.exptime_ == 0) nitem->set_exptime(0);
    else if (request.exptime_ <= kSecondsOneMonth) nitem->set_exptime(now + request.exptime_);// todo overflow
    else nitem->set_exptime(request.exptime_ );
    nitem->set_time(now);
    nitem->set_key(request.key_.c_str(), request.key_.size());
    nitem->set_value(request.value_.c_str(), request.value_.size());
    cache_.add(nitem);
    response.code_ = kSucc;
    return;
  }
  else if (request.op_ == Opt::kDelete)
  {
    Item* item = cache_.find(request.key_.c_str(), false);
    if (NULL != item)
    {
      response.code_ = kFail;
      return;
    }
    else
    {
      cache_.remove(item);
      response.code_ = kSucc;
      return;
    }
  }
  else if (request.op_ == Opt::kIncr || request.op_ == Opt::kDecr)
  {
    Item* item = cache_.find(request.key_.c_str(), true);
    if (NULL != item)
    {
      response.code_ = kFail;
      return;
    }
    uint32_t value = 0;
    if (!digits::convert<uint32_t>(item->value(), value, 10))
    {
      LOG_ERROR << "key = " << request.key_ << " error = convert " << (item->value()) << " to uint32_t fail!"; 
      response.code_ = kError;
      return;
    }
    if (request.op_ == Opt::kIncr) //todo check overflow
      value += request.count_;
    else
      value = (value <= request.count_ ? 0 : value - request.count_) ;
  
    std::string value_str = std::to_string(value);
    item->set_value(value_str.c_str(), value_str.size()); //todo should keep the minmum item can save a uint32_t number
    response.count_ = value;
    return;
  }
  else
  {
    LOG_ERROR << "UNKNOWN op '" << request.op_ << "'";
  }
}

}
}