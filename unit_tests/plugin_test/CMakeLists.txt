
add_executable(plugin main.cpp)
target_link_libraries( plugin optci gtest gtest_main )

add_test( "plugin" plugin )
