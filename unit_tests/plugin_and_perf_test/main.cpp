
#include <optci.h>
#include <gtest/gtest.h>
#include <fstream>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

TEST( SIMPLE, Positive ) {
  int optci_argc = 2;
  char* optci_argv[2];
  optci_argv[0] = "../../bin_optci";
  optci_argv[1] = "config";

  optci_run(optci_argc, optci_argv); 

  fs::path code_path = "optci_example_matrix";

  ifstream in(code_path / "speedup.txt");
  double speedup;
  in >> speedup;

  EXPECT_GE( speedup, 2 );

  EXPECT_EQ( fs::exists( code_path / "perf.data.base"), true );
  EXPECT_EQ( fs::exists( code_path / "perf.data.changed"), true );

}
