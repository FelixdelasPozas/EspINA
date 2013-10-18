# Bound tests
create_test_sourcelist(TEST_SOURCES Segmentation_Tests.cpp # this file is created by this command
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

add_executable(Segmentation_Tests
  ${TEST_SOURCES}
#   ${SUBJECT_MOCS}
#   ${SUBJECT_SOURCES}
  )

target_link_libraries(Segmentation_Tests ${LIB_DEPENDECIES} )

add_test("\"Segmentation: Default Constructor\"" SegmentationǛ_Tests segmentation_)
add_test("\"Segmentation: Save State\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Restore State\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Save Snapshot\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Change Number\"" SegmentationǛ_Tests segmentation_)
add_test("\"Segmentation: Change Category\"" SegmentationǛ_Tests segmentation_)
add_test("\"Segmentation: Register User Modifications\"" SegmentationǛ_Tests segmentation_)
add_test("\"Segmentation: Change Output\"" SegmentationǛ_Tests segmentation_)
add_test("\"Segmentation: Add Extension\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Delete Extension\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Request Extension Information\"" Segmentation_Tests segmentation_)
add_test("\"Segmentation: Request Non Existing Information\"" Segmentation_Tests segmentation_)