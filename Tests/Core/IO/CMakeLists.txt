get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set (TEST_NAME ${COMPONENT_NAME}_test)

# MODIFY WHEN ADDING NEW TESTS
create_test_sourcelist(TESTS  
  ${TEST_NAME}.cpp
  loadWithoutSegFile.cpp
  loadModelFromSegFile.cpp
  saveModelToSegFile.cpp
  )

add_executable(${TEST_NAME} ${TESTS} ${HELPER_SOURCES})
target_link_libraries(${TEST_NAME} ${LIB_DEPENDECIES} )

# Remove the test driver source file
set( TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ${TEST_NAME}.cpp)

set( STACK_DIR "${CMAKE_SOURCE_DIR}/Tests/test images/")

# add tests
foreach(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test("${COMPONENT_NAME}-${TName}" ${TEST_NAME} ${TName} ${STACK_DIR})
endforeach(TEST)
