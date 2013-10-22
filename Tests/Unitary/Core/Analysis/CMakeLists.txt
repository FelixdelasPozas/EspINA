# Analysis tests
create_test_sourcelist(Analysis_Tests Analysis_Tests.cpp # this file is created by this command
  analysis_add_sample.cpp
  analysis_add_channel.cpp
  analysis_add_relation.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(Analysis_Tests
  ${Analysis_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  )
target_link_libraries(Analysis_Tests ${LIB_DEPENDECIES} )

add_test("\"Analysis: Add Sample\""      Analysis_Tests analysis_add_sample)
#add_test("\"Analysis: Add Channel\""     Analysis_Tests analysis_add_channel)
#add_test("\"Analysis: Add Segmentation\"" Analysis_Tests analysis_)
add_test("\"Analysis: Add Relation\""    Analysis_Tests analysis_add_relation)
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
