CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(optci)

SET( CMAKE_BUILD_TYPE DEBUG )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

configure_file( yamlcpp-download/CMakeLists.txt.in yamlcpp-download/CMakeLists.txt)
execute_process( COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} . WORKING_DIRECTORY yamlcpp-download)
execute_process( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY yamlcpp-download )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native" )
SET( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -DGLIBCXX_DEBUG -pthread" )
SET( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++1z" )
SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_C_FLAGS_DEBUG}" )
SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" )

SET( CMAKE_EXPORT_COMPILE_COMMANDS on )

link_directories(${YAMLCPP_LIBRARY_DIRS} ) 
include_directories( ${CMAKE_BINARY_DIR}/yamlcpp-src/include )

add_library(yaml-cpp SHARED IMPORTED GLOBAL)
set_target_properties(yaml-cpp PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/yamlcpp-build/libyaml-cpp.so")

SET( LIBRARIES stdc++fs  hwloc )
SET( SOURCES main.cpp ConfigurationParser.cpp Phase.cpp )

ADD_EXECUTABLE(bin_optci ${SOURCES})
TARGET_LINK_LIBRARIES( bin_optci ${LIBRARIES} )
target_link_libraries( bin_optci ${CMAKE_BINARY_DIR}/yamlcpp-build/libyaml-cpp.a)

ADD_EXECUTABLE( bin_perf_data_converter pdc_main.cpp)

TARGET_LINK_LIBRARIES( bin_perf_data_converter stdc++fs )

ADD_LIBRARY(optci SHARED ${SOURCES})

TARGET_LINK_LIBRARIES( optci ${LIBRARIES} )
set_target_properties( optci PROPERTIES COMPILE_FLAGS "-DBUILDING_LIB=1")

target_link_libraries( optci yaml-cpp )

INCLUDE_DIRECTORIES( "." )

enable_testing()

ADD_SUBDIRECTORY( result_analysis )
ADD_SUBDIRECTORY( unit_tests )
