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
* http module


### example
* echo server
* idle server
* memcached client imitate xmemcached-client
* memcached server