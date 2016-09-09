#include <string>


template<typename T>
T convert(const std::string& str, int base)
{

};

template<>
int convert(const std::string& str, int base)
{
  return 1;
};


void f(const std::string& s)
{

};

int main()
{
  convert(1);
  f("abc");
  return 0;
}