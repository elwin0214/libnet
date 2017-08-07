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
* memcached client/server module (support partial text protocol)

### example
* echo server
* idle server
* memcached client imitate xmemcached-client
* memcached server

### dependent libraries
* [gperftools-2.5](https://github.com/gperftools/gperftools.git)
* [google test](https://github.com/google/googletest)
* [google benchmark](https://github.com/google/benchmark.git)