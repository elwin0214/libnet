#include <libnet/timestamp.h>
#include <stdio.h>
#include <map>

using namespace libnet;

void test_format()
{
  Timestamp now = Timestamp::now();
  fprintf(stdout, "now=%s\n", now.toString().c_str());
};

void test_map()
{
  typedef std::multimap<Timestamp, int> Queue;
  Queue queue;
  Timestamp now = Timestamp::now();
  queue.insert(std::pair<const Timestamp, int>(now, 1));
  fprintf(stdout, "queue.size = %lu\n", queue.size());
};


int main()
{
  test_format();
  test_map();
  return 0;
}