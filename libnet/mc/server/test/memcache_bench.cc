#include <stdio.h>
#include <iostream>
#include <map>
#include <strings.h>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <benchmark/benchmark.h>
#include <libnet/mc/slab.h>
#include <libnet/mc/item.h>
#include <libnet/mc/mem_cache.h>
#include <libnet/logger.h>
#include <libnet/timestamp.h>

using namespace std;
using namespace libnet;
using namespace mc::server;

static int key_size = 7;
static int value_size = 7;

static void Cache_GenKey(benchmark::State& state){
  while (state.KeepRunning())
    for (int j = 0; j < state.range(0); ++j){
      char buf[64]; 
      ::bzero(buf, sizeof(buf));
      ::sprintf(buf, " %d\r\n", 100);
    }
}

static void Now(benchmark::State& state){
  while (state.KeepRunning())
    for (int j = 0; j < state.range(0); ++j){
      uint64_t now = Timestamp::now().secondsValue();
    }
}



static void Slab_Alloc(benchmark::State& state)
{
  while (state.KeepRunning())
  { 
    SlabList sa(SlabOption(state.range(0), 1024, 1.2, 1024 * 1024, true, state.range(1)));
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j)
    {
      int index = -1;
      Item* item = sa.pop(key_size + value_size + 2, index);
      if (NULL == item)
      {
        state.SkipWithError("can not alloc item!");
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}


static void Cache_Alloc(benchmark::State& state)
{
  //log::LogLevel logLevel = log::LogLevel(0);
  //setLogLevel(logLevel);
  while (state.KeepRunning())
  { 
    ConcurrentSlabList slablist(SlabOption(16, 1024, 1.2, 1024 * 1024, true, state.range(1))); 
    MemCache cache(state.range(0), 1.2, std::hash<std::string>(), slablist);
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j)
    {
      Item* item = cache.alloc(key_size + value_size + 2);
      if (NULL == item)
      {
        state.SkipWithError("can not alloc item!");
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

static void STL_Alloc(benchmark::State& state)
{
  typedef std::unordered_map<string,string> umap;  

  while (state.KeepRunning())
  { 
    std::allocator<string> alloc;
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(0); ++j)
    {
      alloc.allocate(sizeof(umap::value_type));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

static void Cache_Add(benchmark::State& state){
  log::LogLevel logLevel = log::LogLevel(5);
  setLogLevel(logLevel);
  const char* value = "1234567";
  function<size_t(string)> hash_func = hash<string>();
  while (state.KeepRunning())
  { 
    //MemCache cache(state.range(0), 1.2, SlabOption(16, 1024, 1.2, 1024 * 1024, true, state.range(1)));
    ConcurrentSlabList slablist(SlabOption(16, 1024, 1.2, 1024 * 1024, true, state.range(1))); 
    MemCache cache(state.range(0), 1.2, std::hash<std::string>(), slablist);
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j)
    {
      char key[64]; 
      ::bzero(key, sizeof(key));
      int len = ::sprintf(key, "key-%d", j);
      Item* item = cache.alloc(len + 7 + 2);
      item->set_key(key, len);
      item->set_hashcode(hash_func(string(key, len)));
      item->set_value(value, 7);
      cache.add(item);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

 

BENCHMARK(Cache_GenKey)->Unit(benchmark::kMicrosecond)->Arg(1<<10)->Arg(1<<20);
BENCHMARK(Now)->Unit(benchmark::kMicrosecond)->Arg(1<<10)->Arg(1<<20);



//BENCHMARK(Slab_Alloc)->Unit(benchmark::kMicrosecond)->Args({8,(1<<30),512})->Args({8, 1<<30, 1<<10})->Args({8, 1<<30, 1<<20})->UseManualTime();
//BENCHMARK(Cache_Alloc)->Unit(benchmark::kMicrosecond)->Args({8, 1<<30, 512})->Args({8, 1<<30, 1<<10})->Args({8, 1<<30, 1<<20})->UseManualTime();
//BENCHMARK(STL_Alloc)->Unit(benchmark::kMicrosecond)->Arg(512)->Arg(1<<10)->Arg(1<<20)->UseManualTime();


//compare resize in hashtable
//BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({10, 1<<30, 512})->Args({10, 1<<30, 1<<10})->Args({10, 1<<30, 1<<20})->UseManualTime();
BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({20, 1<<30, 512})->Args({20, 1<<30, 1<<10})->Args({20, 1<<30, 1<<20})->UseManualTime();
//BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({20, 1<<28, 512})->Args({20, 1<<28, 1<<10})->Args({20, 1<<28, 1<<20})->Args({20, 1<<28, 1<<30})->UseManualTime();
//BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({20, 1<<28, 1<<30})->UseManualTime();

//compare map and unordered map

//compare resize in unordered map<string,string>

// compate construct cost

BENCHMARK_MAIN();  