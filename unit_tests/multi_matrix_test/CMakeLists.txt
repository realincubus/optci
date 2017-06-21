
add_executable(mult_matrix main.cpp)
target_link_libraries( mult_matrix optci gtest gtest_main )

add_test( "mult_matrix" mult_matrix )
