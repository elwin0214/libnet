#include <iostream>
#include "../slab.h"
#include "../item.h"
#include "../memcache.h"
#include <gperftools/profiler.h>
using namespace std;
using namespace libnet;
using namespace memcached::server;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    LOG_ERROR << "<program> <key numbers>" ;
    exit(1);
  }
  int numbers = atoi(argv[1]); 
  MemCache cache(16, 1.2, SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 1024));
  const char* value = "1234567";
  ::ProfilerStart("cpu.prof");
    
  for (int j = 0; j < numbers; ++j)
  {
    char key[64]; 
    ::bzero(key, sizeof(key));
    int len = ::sprintf(key, "key-%d", j);
    Item* item = cache.alloc(len + 7 + 2);
    item->set_key(key, len);
    item->set_value(value, 7);
    cache.add(item);
  }  
  ::ProfilerStop();
}