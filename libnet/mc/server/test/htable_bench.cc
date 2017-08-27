#include <libnet/mc/htable.h>
#include <chrono>
#include <benchmark/benchmark.h>
#include <libnet/logger.h>
#include <unordered_map>
#include <map>

using namespace std;
using namespace libnet;
using namespace mc::server;

std::function<size_t(std::string)> gHash = std::hash<std::string>();
struct Item
{
  Item(std::string key):key_(key),hashcode_(gHash(key))
  {

  }

  std::string key_;
  size_t hashcode_;
};

struct HTableWrapper
{
  HTableWrapper(size_t hashpower, double factor)
    : htable_(hashpower, factor)
  {
    htable_.set_hash(gHash);
    htable_.set_equals([](std::string k1, std::string k2){
      return k1 == k2;
    });

    htable_.set_getkey([](Item* item){
      return item->key_;
    });

    htable_.set_gethash([](Item* item){
      return item->hashcode_;
    });
  }
  HTable<std::string, Item*> htable_;
};


static void HTable_Add(benchmark::State& state){
  log::LogLevel logLevel = log::LogLevel(5);
  setLogLevel(logLevel);

  const char* value = "1234567";
  while (state.KeepRunning())
  { 
    HTableWrapper htable_wrapper(state.range(0), (0 == state.range(1) ? 1.2 : state.range(1)));
    HTable<std::string, Item*>& htable = htable_wrapper.htable_;
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j)
    {
      char key[64]; 
      ::bzero(key, sizeof(key));
      int len = ::sprintf(key, "key-%d", j);
      Item* item = new Item(key);
      htable.addItem(item);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

static void UnOrderedMap_Add(benchmark::State& state){
  typedef std::unordered_map<string,string> umap;  
  
  while (state.KeepRunning())
  {
    umap m(state.range(0));
    m.max_load_factor((0 == state.range(1) ? 1.2 : state.range(1)));
    string value = "1234567";
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(2); ++j){
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

    m.max_load_factor((0 == state.range(1) ? 1.2 : state.range(1)));
    auto start = std::chrono::high_resolution_clock::now();
    int value = 1;
    for (int j = 0; j < state.range(2); ++j){
      m.insert(umap::value_type(j, value));  
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
 


BENCHMARK(HTable_Add)->Unit(benchmark::kMicrosecond)
                     ->Args({10, 0, 1<<10})
                     ->Args({10, 0, 1<<20})
                     ->Args({10, 0,  10 * 1024 *1024 })
                     ->UseManualTime();

BENCHMARK(HTable_Add)->Unit(benchmark::kMicrosecond)
                     ->Args({20, 0, 1<<10})
                     ->Args({20, 0, 1<<20})
                     ->Args({20, 0, 10 * 1024 *1024 })
                     ->UseManualTime();

BENCHMARK(HTable_Add)->Unit(benchmark::kMicrosecond)
                     ->Args({10, 3, 1<<10})
                     ->Args({10, 3, 1<<20})
                     ->Args({10, 3, 10 * 1024 *1024 })
                     ->UseManualTime();

BENCHMARK(UnOrderedMap_Add)->Unit(benchmark::kMicrosecond)
                           ->Args({1<<10, 0, 1<<10})
                           ->Args({1<<10, 0, 1<<20})
                           ->Args({1<<10, 0, 10 * 1024 *1024})
                           ->UseManualTime();

BENCHMARK(UnOrderedMap_Add)->Unit(benchmark::kMicrosecond)
                           ->Args({1<<20, 0, 1<<10})
                           ->Args({1<<20, 0, 1<<20})
                           ->Args({1<<20, 0, 10 * 1024 *1024})
                           ->UseManualTime();

BENCHMARK(UnOrderedMap_Add)->Unit(benchmark::kMicrosecond)
                           ->Args({1<<10, 3, 1<<10})
                           ->Args({1<<10, 3, 1<<20})
                           ->Args({1<<10, 3, 10 * 1024 *1024})
                           ->UseManualTime();

BENCHMARK(UnOrderedMap_AddInt)->Unit(benchmark::kMicrosecond)
                              ->Args({1<<10, 0, 512})
                              ->Args({1<<10, 0, 1<<10})
                              ->Args({1<<10, 0, 1<<20})
                              ->UseManualTime();

BENCHMARK(Map_Add)->Unit(benchmark::kMicrosecond)
                  ->Arg(512)
                  ->Arg(1<<10)
                  ->Arg(1<<20)
                  ->Arg(10*1024*1024)
                  ->Arg(20*1024*1024)
                  ->UseManualTime();;


BENCHMARK_MAIN();  
