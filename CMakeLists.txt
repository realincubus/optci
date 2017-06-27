CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(optci)

SET( CMAKE_BUILD_TYPE DEBUG )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native" )
SET( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -DGLIBCXX_DEBUG " )
SET( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++1z" )
SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_C_FLAGS_DEBUG}" )
SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" )

#SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
#SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

SET( CMAKE_EXPORT_COMPILE_COMMANDS on )

SET( LIBRARIES stdc++fs yaml-cpp hwloc )
SET( SOURCES main.cpp ConfigurationParser.cpp Phase.cpp )

ADD_EXECUTABLE(bin_optci ${SOURCES})
TARGET_LINK_LIBRARIES( bin_optci ${LIBRARIES} )

ADD_EXECUTABLE( bin_perf_data_converter pdc_main.cpp)

TARGET_LINK_LIBRARIES( bin_perf_data_converter stdc++fs )

ADD_EXECUTABLE( bin_ra results_accumulator.cpp  results_analyzer.cpp )
TARGET_LINK_LIBRARIES( bin_ra yaml-cpp )

# RESULT ANALYSIS
ADD_EXECUTABLE( bin_rana results_analyzer.cpp result_analysis_parser.cpp result_analysis_plot.cpp )
TARGET_LINK_LIBRARIES( bin_rana yaml-cpp )
set_target_properties( bin_rana PROPERTIES COMPILE_FLAGS "-DBUILDING_EXE=1")

ADD_LIBRARY( ra SHARED results_accumulator.cpp results_analyzer.cpp result_analysis_parser.cpp result_analysis_plot.cpp )
TARGET_LINK_LIBRARIES( ra yaml-cpp )
set_target_properties( ra PROPERTIES COMPILE_FLAGS "-DBUILDING_LIB=1")


ADD_LIBRARY(optci SHARED ${SOURCES})

TARGET_LINK_LIBRARIES( optci ${LIBRARIES} )
set_target_properties( optci PROPERTIES COMPILE_FLAGS "-DBUILDING_LIB=1")

INCLUDE_DIRECTORIES( "." )

enable_testing()

ADD_SUBDIRECTORY( unit_tests )
