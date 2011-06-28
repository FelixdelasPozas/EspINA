

create_test_sourcelist(TESTS  
  ConnectedThresholdImageFilter_tests.cpp
  ConnectedThresholdImageFilter_compilation.cpp
  ConnectedThresholdImageFilter_invalidSeed.cpp
  )

add_executable(ConnectedThresholdImageFilter_tests ${TESTS})
target_link_libraries(ConnectedThresholdImageFilter_tests ${LIB_DEPENDECIES})

# Remove the test driver source file
set( TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ConnectedThresholdImageFilter_tests.cpp)

# add tests
FOREACH(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test(${TName} ConnectedThresholdImageFilter_tests ${TName} ${STACK_DIR}/peque.mha)
#         SET(EXE_NAME Cajal_${TEST})
#         ADD_EXECUTABLE(${EXE_NAME} ${TEST}.cpp)
#         TARGET_LINK_LIBRARIES(${EXE_NAME} ${LIB_DEPENDECIES})
#         ADD_TEST(Cajal-${TEST} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME} ${STACK_DIR}/peque.mha)
ENDFOREACH(TEST)