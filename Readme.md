A simple noblocking TCP Server imitate muduo.

### environment

* g++
* c++11 (function、shared_ptr、unique_ptr、move)

### build

```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug/Release ..
make 
make test
```

### feature
* select/poll/epoll
* use std::map to achieve the timer
* http server module
* memcached client/server/proxy module (support partial text protocol)

### example
* [echo server](https://github.com/elwin0214/libnet/blob/master/example/echo/echo_server_test.cc)
* [idle server](https://github.com/elwin0214/libnet/blob/master/example/idle/idle_server_test.cc)
* [tcp proxy server](https://github.com/elwin0214/libnet/blob/master/example/proxy/tcp_proxy.cc)
* [http server](https://github.com/elwin0214/libnet/blob/master/libnet/http/test/httpserver_test.cc)
* [memcached client](https://github.com/elwin0214/libnet/blob/master/libnet/mc/client/test/async_client_bench.cc)
* [memcached server](https://github.com/elwin0214/libnet/blob/master/libnet/mc/server/test/memcached_server_bench.cc)

### dependent libraries
* [gperftools-2.5](https://github.com/gperftools/gperftools.git)
* [google test](https://github.com/google/googletest)
* [google benchmark](https://github.com/google/benchmark.git)