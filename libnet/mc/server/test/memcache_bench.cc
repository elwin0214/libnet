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

static void Slab_Alloc(benchmark::State& state)
{
  while (state.KeepRunning())
  { 
    SlabArray sa(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 1024));
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(0); ++j)
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
  while (state.KeepRunning())
  { 
    MemCache cache(8, 1.2, SlabOption(16, 1024, 1.2, 100 * 1024 * 1024, true, 1024 * 1024 * 1024));
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(0); ++j)
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
  const char* value = "1234567";
  while (state.KeepRunning())
  { 
    MemCache cache(state.range(0), 1.2, SlabOption(16, 1024, 1.2, 1024 * 1024, true,  1024 * 1024 * 1024));
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(1); ++j)
    {
      char key[64]; 
      ::bzero(key, sizeof(key));
      int len = ::sprintf(key, "key-%d", j);
      Item* item = cache.alloc(len + 7 + 2);
      item->set_key(key, len);
      item->set_value(value, 7);
      cache.add(item);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

static void Map_Add(benchmark::State& state){
  
  const char* value = "1234567";
  while (state.KeepRunning())
  {
    map<string,string> m;
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(0); ++j){
      char key[64]; 
      ::bzero(key, sizeof(key));
      int len = ::sprintf(key, "key-%d", j);
      m.emplace(key, value);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

static void UnOrderedMap_AddString(benchmark::State& state){
  typedef std::unordered_map<string,string> umap;  
  
  while (state.KeepRunning())
  {
    umap m(state.range(0));
    m.max_load_factor(1.2);
    string value = "1234567";
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(1); ++j){
      char key[64]; 
      ::bzero(key, sizeof(key));
      sprintf(key, "key-%d", j);
      m.insert(umap::value_type(string(key), value));  
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  } 
}

static void UnOrderedMap_AddInt(benchmark::State& state){
  typedef std::unordered_map<int,int> umap;  
  
  while (state.KeepRunning())
  {
    umap m(state.range(0));

    m.max_load_factor(1.2);
    auto start = std::chrono::high_resolution_clock::now();
    int value = 1;
    for (int j = 0; j < state.range(1); ++j){
      m.insert(umap::value_type(j, value));  
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}
 

BENCHMARK(Cache_GenKey)->Unit(benchmark::kMicrosecond)->Arg(1<<10)->Arg(1<<20);


BENCHMARK(Slab_Alloc)->Unit(benchmark::kMicrosecond)->Arg(512)->Arg(1<<10)->Arg(1<<20)->UseManualTime();
BENCHMARK(Cache_Alloc)->Unit(benchmark::kMicrosecond)->Arg(512)->Arg(1<<10)->Arg(1<<20)->UseManualTime();
BENCHMARK(STL_Alloc)->Unit(benchmark::kMicrosecond)->Arg(512)->Arg(1<<10)->Arg(1<<20)->UseManualTime();


//compare resize in hashtable
BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({10, 512})->Args({10, 1<<10})->Args({10, 1<<20})->UseManualTime();
BENCHMARK(Cache_Add)->Unit(benchmark::kMicrosecond)->Args({24, 512})->Args({24, 1<<10})->Args({24, 1<<20})->UseManualTime();

//compare map and unordered map
BENCHMARK(Map_Add)->Unit(benchmark::kMicrosecond)->Arg(512)->Arg(1<<10)->Arg(1<<20)->UseManualTime();;

//compare resize in unordered map<string,string>
BENCHMARK(UnOrderedMap_AddString)->Unit(benchmark::kMicrosecond)->Args({1<<6, 512})->Args({1<<6, 1<<10})->Args({1<<6,1<<20})->UseManualTime();//->Arg(1<<30);
BENCHMARK(UnOrderedMap_AddString)->Unit(benchmark::kMicrosecond)->Args({1<<24, 512})->Args({1<<24, 1<<10})->Args({1<<24,1<<20})->UseManualTime();

// compare resize in unordered map<int,int>
BENCHMARK(UnOrderedMap_AddInt)->Unit(benchmark::kMicrosecond)->Args({1<<6, 512})->Args({1<<6, 1<<10})->Args({1<<6,1<<20})->UseManualTime();
BENCHMARK(UnOrderedMap_AddInt)->Unit(benchmark::kMicrosecond)->Args({1<<24, 512})->Args({1<<24, 1<<10})->Args({1<<24,1<<20})->UseManualTime();
// compate construct cost
//BENCHMARK(UnOrderedMap_Construct)->Unit(benchmark::kMicrosecond)->Args({1<<6, 512})->Args({1<<6, 1<<10})->Args({1<<6,1<<20});
//BENCHMARK(UnOrderedMap_Construct)->Unit(benchmark::kMicrosecond)->Args({1<<24, 512})->Args({1<<24, 1<<10});

BENCHMARK_MAIN();















  