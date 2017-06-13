
add_executable(scaling main.cpp)
target_link_libraries( scaling optci gtest gtest_main )

add_test( "scaling" scaling )
