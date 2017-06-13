
#include <optci.h>
#include <gtest/gtest.h>
#include <fstream>

using namespace std;

TEST( SIMPLE, Positive ) {
  int optci_argc = 2;
  char* optci_argv[2];
  optci_argv[0] = "../../bin_optci";
  optci_argv[1] = "config";

  optci_run(optci_argc, optci_argv); 

  ifstream in("optci_example_matrix/speedup.txt");
  double speedup;
  in >> speedup;

  EXPECT_GE( speedup, 2 );

}
