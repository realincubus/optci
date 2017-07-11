
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

  for (int i = 0; i < 3; ++i){
    std::stringstream str;
    str << "SOCKETS=1" << endl;
    str << "CORES_PER_SOCKET=" << i+1 << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TIME=" << 1.0/(i+1) << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }
  for (int s = 1; s < 3; ++s){
    std::stringstream str;
    str << "SOCKETS=" << s+1 << endl;
    str << "CORES_PER_SOCKET=1" << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TIME=" << (1.0/(s+1))+0.1 << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }
  
  write_yaml_tree( root, std::cout ); 

  auto analysis_results = analyze_configuration_matrix( root ); 

  auto& cores_result = analysis_results[0];

  EXPECT_EQ( cores_result.configuration.name, "CORES_PER_SOCKET" );
  EXPECT_EQ( (bool)cores_result.no_change, false );
  EXPECT_EQ( (bool)cores_result.monotonic_decrease, true );

  auto& sockets_result = analysis_results[1];

  EXPECT_EQ( sockets_result.configuration.name, "SOCKETS" );
  EXPECT_EQ( (bool)sockets_result.no_change, false );
  EXPECT_EQ( (bool)sockets_result.monotonic_decrease, true );
  
}

TEST( CrossAnalysisTest, Positive ){

  auto root = build_yaml_root();

  for (int s = 0; s < 3; ++s){
    for (int i = 0; i < 3; ++i){
      std::stringstream str;
      str << "SOCKETS=" << s+1 << endl;
      str << "CORES_PER_SOCKET=" << i+1 << endl;
      str << "TIME=" << 1.0/((i+1)+(s+1)*3) << endl;

      std::istringstream istr( str.str() );

      root = add_yaml_configuration_from_stream( root, str );
    }
  }

  write_yaml_tree( root, std::cout ); 

  analyze_configuration_matrix( root ); 
}

TEST( LargeCrossAnalysisTest, Positive ){

  auto root = build_yaml_root();

  for (int m = 0; m < 4; ++m){
    for (int s = 0; s < 2; ++s){
      for (int c = 0; c < 6; ++c){
        for (int t = 0; t < 2; ++t){
          std::stringstream str;
          str << "MACHINES=" << m+1 << endl;
          str << "SOCKETS=" << s+1 << endl;
          str << "CORES_PER_SOCKET=" << c+1 << endl;
          str << "THREADS_PER_CORE=" << t+1 << endl;
          str << "TIME=" << 1.0/((c+1)+(s+1)*3) << endl;

          std::istringstream istr( str.str() );

          root = add_yaml_configuration_from_stream( root, str );
        }
      }
    }
  }
  write_yaml_tree( root, std::cout ); 

  auto results = analyze_configuration_matrix( root ); 

  for( auto&& result : results ){
    std::cout << "result " << result.configuration << std::endl;
  }
  
}

// check that a change in time caused by jitter does
// not result in a optimization hint
TEST( NoChangeTest, Positive ){

  auto root = build_yaml_root();

  for (int i = 0; i < 3; ++i){
    std::stringstream str;
    str << "SOCKETS=1" << endl;
    str << "CORES_PER_SOCKET=" << i+1 << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TIME=" << 1.0-(i/1000.0) << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }
  
  write_yaml_tree( root, std::cout ); 

  analyze_configuration_matrix( root ); 
}


// check that a change in time caused by jitter does
// not result in a optimization hint
TEST( DoesScaleTest, Positive ){

  auto root = build_yaml_root();

  // values for non scaling runs
  for (int i = 0; i < 3; ++i){
    std::stringstream str;
    str << "SOCKETS=1" << endl;
    str << "CORES_PER_SOCKET=" << i+1 << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TYPE=BASE" << endl;
    str << "TIME=" << 1.0 << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }

  // values for changed scaling runs
  for (int i = 0; i < 3; ++i){
    std::stringstream str;
    str << "SOCKETS=1" << endl;
    str << "CORES_PER_SOCKET=" << i+1 << endl;
    str << "THREADS_PER_CORE=1" << endl;
    str << "TYPE=CHANGED" << endl;
    str << "TIME=" << 1.0/(i+1) << endl;

    std::istringstream istr( str.str() );

    root = add_yaml_configuration_from_stream( root, str );
  }
  
  write_yaml_tree( root, std::cout ); 

  Options options;

  options.target_axis = "TIME";
  options.compare = "TYPE = BASE / TYPE = CHANGED";
  options.greater = "2";

  auto ret = compare_data( root, options );

  EXPECT_EQ ( ret , EXIT_SUCCESS );

}













