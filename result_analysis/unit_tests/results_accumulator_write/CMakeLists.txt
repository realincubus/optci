
add_executable(results_accumulator_write main.cpp)
target_link_libraries( results_accumulator_write rana gtest gtest_main )

add_test( "results_accumulator_write" results_accumulator_write )
