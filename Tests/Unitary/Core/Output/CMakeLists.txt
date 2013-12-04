# Bound tests
create_test_sourcelist(TEST_SOURCES Output_Tests.cpp # this file is created by this command
  output_valid_output.cpp
  output_invalid_output.cpp
  output_update_filter.cpp
  output_replace_data.cpp
  output_remove_data.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(Output_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/BinaryMask.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  output_testing_support.cpp
  )

target_link_libraries(Output_Tests ${LIB_DEPENDECIES} )

add_test("\"Output: Create Invalid Output\"" Output_Tests output_invalid_output)
add_test("\"Output: Create Valid Output\""   Output_Tests output_valid_output)
add_test("\"Output: Update Filter\""         Output_Tests output_update_filter)
add_test("\"Output: Replace Data\""          Output_Tests output_replace_data)
add_test("\"Output: Remove Data\""           Output_Tests output_remove_data)