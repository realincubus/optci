
add_executable(all main.cpp)
target_link_libraries( all optci gtest gtest_main )

add_test( "all" all )
