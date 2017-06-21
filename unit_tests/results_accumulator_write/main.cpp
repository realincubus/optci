
#include <gtest/gtest.h>
#include <vector>
#include "results_accumulator.hpp"
#include "results_analyzer.hpp"
#include <sstream>

using namespace std;

TEST( ReadIniTest, Positive ){

  std::stringstream str;
  str << "KEY=VALUE" << endl;

  std::istringstream istr( str.str() );

  auto root = build_yaml_root();
  root = add_yaml_configuration_from_stream(root, str );
}

// TODO 
#if 0
TEST( ReadYAMLTest, Positive ){

  std::stringstream str;
  str << "KEY=VALUE" << endl;

  std::istringstream istr( str.str() );

  auto root = add_yaml_configuration_from_stream( str );
}
#endif

TEST( AnalysisTest, Positive ){

  auto root = build_yaml_root();

  for (int i = 0; i < 2; ++i){
      
    std::stringstream str;
    str << "SOCKETS=1" << endl;
    str << "CORES=" << i+1 << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TIME=" << 1.0/(i+1) << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }
  
  write_yaml_tree( root, std::cout ); 

  analyze_configuration_matrix( root ); 
}



