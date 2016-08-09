A simple noblocking TCP Server imitate muduo.

### environment

* g++
* c++11

### build

```shell
mkdir build
cmake -DCMAKE_BUILD_TYPE=Debug/Release ..
make 
make test
```

### feature
* select/poll/epoll
* use std::map to achieve the timer
* http module


### example
* echo
* memcached client imitate xmemcached-client