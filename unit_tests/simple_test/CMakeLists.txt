
add_executable(simple main.cpp)
target_link_libraries( simple optci gtest gtest_main )

add_test( "simple" simple )
