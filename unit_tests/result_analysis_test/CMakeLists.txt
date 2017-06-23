
add_executable(result_analysis main.cpp)
target_link_libraries( result_analysis optci gtest gtest_main )

add_test( "result_analysis" result_analysis )
