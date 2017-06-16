
add_executable(auto_scaling main.cpp)
target_link_libraries( auto_scaling optci gtest gtest_main )

add_test( "auto_scaling" auto_scaling )
