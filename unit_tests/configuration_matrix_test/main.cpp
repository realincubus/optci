
#include <gtest/gtest.h>
#include "ConfigurationMatrix.hpp"

TEST( ConfigurationMatrixTest, Positive ){
  ConfigurationMatrix m;
  Axis cores("CORES_PER_SOCKET");
  cores.add_value("1");
  cores.add_value("2");
  cores.add_value("4");
  cores.add_value("8");

  Axis sockets("SOCKETS");
  sockets.add_value("0-0");
  sockets.add_value("0-1");

  m.add_axis(sockets);
  m.add_axis(cores);

  m.for_each_configuration( [](auto a){ 
     
  });

}

