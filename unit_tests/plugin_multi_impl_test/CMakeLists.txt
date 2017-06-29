
add_executable(plugin_multi_impl main.cpp)
target_link_libraries( plugin_multi_impl optci gtest gtest_main )

add_test( "plugin_multi_impl" plugin_multi_impl )
