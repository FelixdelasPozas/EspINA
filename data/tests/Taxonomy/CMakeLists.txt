INCLUDE_DIRECTORIES (
  ../..
  )

get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set (TEST_NAME ${COMPONENT_NAME}_test)
# 
# create_test_sourcelist(TESTS  
#    ${TEST_NAME}.cpp
#   insertInANonExistentSupElement.cpp  
#   repeatedElement.cpp
#   writing_reading.cpp
#   )

# add_executable(${TEST_NAME} ${TESTS})
# target_link_libraries(${TEST_NAME} ${LIB_DEPENDECIES})

# Remove the test driver source file
set( TestToRun 
  insertInANonExistentSupElement.cpp  
  repeatedElement.cpp
  writing_reading.cpp
  remove_parent.cpp
#   ${TESTS} 
)

# add_executable(insertInANonExistentSupElement insertInANonExistentSupElement.cpp)
# target_link_libraries(insertInANonExistentSupElement ${LIB_DEPENDECIES})

# add_test(test-insertInANonExistentSupElement insertInANonExistentSupElement ${STACK_DIR})
# list (REMOVE_ITEM TestToRun ${TEST_NAME}.cpp)

# add tests
FOREACH(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_executable(${COMPONENT_NAME}-${TName} ${TEST})
  target_link_libraries(${COMPONENT_NAME}-${TName} ${LIB_DEPENDECIES})
  add_test("${COMPONENT_NAME}-${TName}" ${COMPONENT_NAME}-${TName} ${STACK_DIR})

#   add_test(${COMPONENT_NAME}-${TName} ${TEST_NAME} ${TName} STACK_DIR)
ENDFOREACH(TEST)
