

SET( SOURCES results_accumulator.cpp results_analyzer.cpp result_analysis_parser.cpp result_analysis_plot.cpp )
# RESULT ANALYSIS
ADD_EXECUTABLE( bin_rana ${SOURCES}  )
TARGET_LINK_LIBRARIES( bin_rana yaml-cpp )
set_target_properties( bin_rana PROPERTIES COMPILE_FLAGS "-DBUILDING_EXE=1")

ADD_LIBRARY( rana SHARED ${SOURCES} )
TARGET_LINK_LIBRARIES( rana yaml-cpp )
set_target_properties( rana PROPERTIES COMPILE_FLAGS "-DBUILDING_LIB=1")

INCLUDE_DIRECTORIES( . )

ADD_SUBDIRECTORY(unit_tests)
