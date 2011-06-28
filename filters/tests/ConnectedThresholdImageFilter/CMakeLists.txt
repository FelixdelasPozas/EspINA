find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

INCLUDE_DIRECTORIES (
  ../..
  ${QT_QTCORE_INCLUDE_DIR}
  )

create_test_sourcelist(TESTS  
  ConnectedThresholdImageFilter_tests.cpp
  pipeline.cpp
  invalidSeed.cpp
  )

add_executable(ConnectedThresholdImageFilter_tests ${TESTS})
target_link_libraries(ConnectedThresholdImageFilter_tests ${LIB_DEPENDECIES} ${QT_QTCORE_LIBRARY})

# Remove the test driver source file
set( TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ConnectedThresholdImageFilter_tests.cpp)

get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# add tests
message(STATUS "STACKDIR " ${STACK_DIR})
FOREACH(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test("${COMPONENT_NAME}-${TName}" ConnectedThresholdImageFilter_tests ${TName} ${STACK_DIR})
ENDFOREACH(TEST)
