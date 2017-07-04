
#include <numeric>
#include <iostream>
#include <memory>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char** argv){

  auto size = stoi(argv[1]);
  auto max_iterations = stoi(argv[2]);

  int X = size;
  int Y = size;
  unique_ptr<double[]> matrix (new double[Y*X]); 

  unique_ptr<double[]> inputVector (new double[X]);
  unique_ptr<double[]> targetVector (new double[Y]);

  std::iota(&matrix[0],&matrix[Y*X],0);
  std::iota(&inputVector[0],&inputVector[X],0);
  std::fill(&targetVector[0], &targetVector[Y], 0); 

  auto start = std::chrono::high_resolution_clock::now();

  int iterations = max_iterations;

  while( iterations-- ) {
    for ( int y = 0; y < Y; y++ ){
      double sum = 0;
      for ( int x = 0; x < X; x++ ) { 
        sum += matrix[x+y*X] * inputVector[x];
      }   
      targetVector[y] = sum;
    }   
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto ms = std::chrono::duration_cast<milliseconds>(end - start).count();
  std::cout << "one mult takes " << ms / max_iterations << " ms val " << targetVector[0] << std::endl;
  std::cout << "total time " << ms << " ms" << std::endl;

  return 0;
}
