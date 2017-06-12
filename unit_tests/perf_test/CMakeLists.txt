
add_executable(perf main.cpp)
target_link_libraries( perf optci gtest gtest_main )

add_test( "perf" perf )
