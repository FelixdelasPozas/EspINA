create_test_sourcelist(TEST_SOURCES Sample_Tests.cpp # this file is created by this command
  sample_set_name.cpp
  sample_set_bounds.cpp
  sample_set_position.cpp
  sample_save_state.cpp
  sample_restore_state.cpp
  sample_save_snapshot.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")

include_directories(
  ${SUBJECT_DIR}
  )

add_executable(Sample_Tests "" ${TEST_SOURCES})

target_link_libraries(Sample_Tests ${CORE_DEPENDECIES} )

add_test("\"Sample: Set Name\""      Sample_Tests sample_set_name)
add_test("\"Sample: Set Bounds\""    Sample_Tests sample_set_bounds)
add_test("\"Sample: Set Position\""  Sample_Tests sample_set_position)
# add_test("\"Sample: Save State\""    Sample_Tests sample_save_state)
# add_test("\"Sample: Restore State\"" Sample_Tests sample_restore_state)
# add_test("\"Sample: Save Snapshot\"" Sample_Tests sample_save_snapshot)
