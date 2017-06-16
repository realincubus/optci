
add_executable(configuration_parser main.cpp)
target_link_libraries( configuration_parser optci gtest gtest_main hwloc )

add_test( "configuration_parser" configuration_parser )
