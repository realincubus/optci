
#include <numeric>
#include <iostream>
#include <memory>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

struct Circle {
  public:
    Circle( double _diameter ) :
      diameter(_diameter)
  {
  }
  double diameter;
};

int main(int argc, char** argv){


  for (int j = 0; j < 1000; ++j){
    std::vector<Circle> circles;
    for (int i = 0; i < 1000000; ++i){
      circles.push_back( Circle( i ) );
    }
  }
  
  return 0;
}
