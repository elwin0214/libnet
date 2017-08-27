#include <stdio.h>
#include <iostream>
#include <map>
#include <strings.h>
#include <chrono>
#include <benchmark/benchmark.h>
#include <libnet/mc/item.h>
#include <libnet/mc/hashtable.h>
#include <libnet/logger.h>
#include <libnet/timestamp.h>

using namespace std;
using namespace libnet;
using namespace mc::server;

static void HashTable_Add(benchmark::State& state){
  log::LogLevel logLevel = log::LogLevel(5);
  setLogLevel(logLevel);
  const char* value = "1234567";
  while (state.KeepRunning())
  { 
    MemCache cache(state.range(0), 1.2, SlabOption(16, 1024, 1.2, 1024 * 1024, true, state.range(1)));
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j)
    {
      char key[64]; 
      ::bzero(key, sizeof(key));
      int len = ::sprintf(key, "key-%d", j);
      Item* item = new 
      item->set_key(key, len);
      item->set_value(value, 7);
      cache.add(item);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}
