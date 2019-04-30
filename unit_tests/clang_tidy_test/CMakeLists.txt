
add_executable(tidy main.cpp)
target_link_libraries( tidy optci gtest gtest_main )

add_test( "tidy" tidy )
