get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set (TEST_NAME ${COMPONENT_NAME}_test)

set (HELPER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/InsertRowsTest.cpp
  )

# MODIFY WHEN ADDING NEW TESTS
create_test_sourcelist(TESTS
  ${TEST_NAME}.cpp
  model_InsertRows.cpp
  )

add_executable(${TEST_NAME} ${TESTS} ${HELPER_SOURCES})
target_link_libraries(${TEST_NAME} ${LIB_DEPENDECIES})

# Remove the test driver source file
set( TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ${TEST_NAME}.cpp)

# add tests
foreach(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test("${COMPONENT_NAME}-${TName}" ${TEST_NAME} ${TName} ${STACK_DIR})
endforeach(TEST)

add_subdirectory( Proxies )
