#include <benchmark/benchmark.h>
#include <libnet/buffer.h>
#include <gtest/gtest.h>
#include <iostream>
#include <libnet/mc/message.h>
#include <libnet/mc/request_codec.h>
#include <libnet/mc/response_codec.h>

using namespace std;
using namespace libnet;
using namespace mc::msg;

static void Request_Decode(benchmark::State& state)
{
  while (state.KeepRunning())
  { 
    int n = 1;
    RequestCodec codec;
    Buffer wb(0, 1024);
    Message req(kSet, "name", "bob", 0, 120);
    for (int i = 0; i < n; i++)
    {
      codec.encode(req, wb);
    }
    string str = wb.toString();
    //cout << wb.toString() << endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < state.range(0); ++j)
    {
      Buffer rb(0, 1024);
      rb.append(str);
      for (int i = 0; i < n; i++)
      {
        Message* req = new Message();
        bool b = (codec.decode(*req, rb));
        //cout << b << endl;
        ASSERT_TRUE(b);
        delete req;
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}

BENCHMARK(Request_Decode)->Unit(benchmark::kMicrosecond)->Arg(1<<10)->Arg(1<<20)->UseManualTime();
BENCHMARK_MAIN();