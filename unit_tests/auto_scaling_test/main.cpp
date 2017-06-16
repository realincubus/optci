
#include <optci.h>
#include <gtest/gtest.h>
#include <fstream>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

TEST( AUTO_SCALING_TEST, Positive ) {
  int optci_argc = 2;
  char* optci_argv[2];

  // TODO extract function in main code to prepare environment
  try {
    fs::remove("source.this");
  }catch(...) {

  }
  try {
    fs::remove_all("artifacts");
  }catch(...) {

  }

  optci_argv[0] = "../../bin_optci";
  optci_argv[1] = "config";

  optci_run(optci_argc, optci_argv); 

  ifstream in("optci_example_matrix/speedup.txt");
  double speedup;
  in >> speedup;
  in >> speedup;
  in >> speedup;
  in >> speedup;

  in >> speedup;
  in >> speedup;
  in >> speedup;
  in >> speedup;

  // the last value in the file sould have the speedup of 2 sockets * 12 cores
  EXPECT_GE( speedup, 2 );

}
