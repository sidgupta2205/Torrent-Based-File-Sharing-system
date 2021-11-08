#include<iostream>
using namespace std;
int main() {
   if (__cplusplus == 201703L)
      std::cout << "C++17" << endl;
   else if (__cplusplus == 201402L)
      std::cout << "C++14" << endl;
   else if (__cplusplus == 201103L)
      std::cout << "C++11" << endl;
   else if (__cplusplus == 199711L)
      std::cout << "C++98" << endl;
   else
      std::cout << "pre-standard C++" << endl;
}