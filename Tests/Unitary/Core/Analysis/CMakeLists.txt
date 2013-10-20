# Analysis tests
create_test_sourcelist(Analysis_Tests Analysis_Tests.cpp # this file is created by this command
  analysis_add_sample.cpp
)

set(SUBJECT_DIR ${CORE_DIR}/Analysis)

include_directories(
  ${SUBJECT_DIR}
  )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Analysis_Tests
  ${Analysis_Tests}
  ${SUBJECT_DIR}/Analysis.cpp
  )
target_link_libraries(Analysis_Tests ${LIB_DEPENDECIES} )

add_test("\"Analysis: Add Sample\"" Analysis_Tests analysis_add_sample)
#add_test("\"Analysis: Add Channel\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Segmentation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Relations\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Existing Sample\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Existing Channel\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Existing Segmentation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Add Existing Relation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Sample\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Channel\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Segmentation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Relation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Non Existing Sample\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Non Existing Channel\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Non Existing Segmentation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Remove Non Existing Relation\"" Analysis_Tests analysis_)
#add_test("\"Analysis: Set Classification\"" Analysis_Tests analysis_)
