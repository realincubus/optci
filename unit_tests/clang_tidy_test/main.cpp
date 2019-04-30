
#include <optci.h>
#include <gtest/gtest.h>
#include <fstream>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;

TEST( SIMPLE, Positive ) {
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

}
