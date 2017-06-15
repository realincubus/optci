
add_executable(configuration_matrix main.cpp)
target_link_libraries( configuration_matrix optci gtest gtest_main )

add_test( "configuration_matrix" configuration_matrix )
