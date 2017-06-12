
#include <gtest/gtest.h>
#include <vector>
#include "ConfigurationMatrix.hpp"

TEST( ConfigurationMatrixTest, Positive ){
  ConfigurationMatrix m;
  Axis cores("CORES_PER_SOCKET");
  cores.add_value("1");
  cores.add_value("2");
  cores.add_value("4");
  cores.add_value("8");

  Axis sockets("SOCKETS");
  sockets.add_value("0");
  sockets.add_value("1");

  m.add_axis(sockets);
  m.add_axis(cores);

  std::vector<Configuration> configs;
  m.for_each_configuration( [&](auto a){ 
    configs.push_back(a);
  });

#if 0
  // recreate
  int ctr = 0;
  int pctr = 0;
  for( auto&& config : configs ){
    pctr = 0;
    for( auto&& pair : config ){
      std::cout << "EXPECT_EQ( configs[" << ctr << "][" << pctr++ << "].second, \"" << pair.second << "\" );" << std::endl;
    }
    ctr++;
  }
#endif

  EXPECT_EQ( configs[0][0].second, "0");
  EXPECT_EQ( configs[0][1].second, "1");
  EXPECT_EQ( configs[1][0].second, "0");
  EXPECT_EQ( configs[1][1].second, "2");
  EXPECT_EQ( configs[2][0].second, "0");
  EXPECT_EQ( configs[2][1].second, "4");
  EXPECT_EQ( configs[3][0].second, "0");
  EXPECT_EQ( configs[3][1].second, "8");
  EXPECT_EQ( configs[4][0].second, "1");
  EXPECT_EQ( configs[4][1].second, "1");
  EXPECT_EQ( configs[5][0].second, "1");
  EXPECT_EQ( configs[5][1].second, "2");
  EXPECT_EQ( configs[6][0].second, "1");
  EXPECT_EQ( configs[6][1].second, "4");
  EXPECT_EQ( configs[7][0].second, "1");
  EXPECT_EQ( configs[7][1].second, "8");


}

TEST( PhaseTest, Positive ){

}
