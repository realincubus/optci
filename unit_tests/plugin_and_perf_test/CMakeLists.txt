
add_executable(plugin_and_perf main.cpp)
target_link_libraries( plugin_and_perf optci gtest gtest_main )

add_test( "plugin_and_perf" plugin_and_perf )
