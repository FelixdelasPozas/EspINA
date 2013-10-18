# Bound tests
create_test_sourcelist(TEST_SOURCES Output_Tests.cpp # this file is created by this command
  analysis_add_sample.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")

include_directories(
  ${SUBJECT_DIR}
  )

# qt4_wrap_cpp(SUBJECT_MOCS
#   ${SUBJECT_DIR}/Category.h
#   ${SUBJECT_DIR}/Data.h
# )

# set(SUBJECT_SOURCES
#   ${SUBJECT_DIR}/Classification.cpp
#   ${SUBJECT_DIR}/Category.cpp
# )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Output_Tests
  ${TEST_SOURCES}
#   ${SUBJECT_MOCS}
#   ${SUBJECT_SOURCES}
  )

target_link_libraries(Output_Tests ${LIB_DEPENDECIES} )

add_test("\"Output: Create Output\"" Output_Tests output_add_sample)
add_test("\"Output: Create Invalid Output\"" Output_Tests output_add_sample)
add_test("\"Output: Create Invalid Output\"" Output_Tests output_add_sample)