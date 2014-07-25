# Analysis Merge tests
create_test_sourcelist(AnalysisMerge_Tests AnalysisMerge_Tests.cpp # this file is created by this command
  analysis_merge_merge_analyses_without_classification.cpp
  analysis_merge_merge_analyses_with_one_classification.cpp
  analysis_merge_merge_analyses_with_two_classifications.cpp
  analysis_merge_merge_analyses_with_category_overlaping.cpp
  analysis_merge_merge_analyses_with_same_sample.cpp
  analysis_merge_merge_analyses_with_same_channel.cpp
  analysis_merge_merge_analyses_input_substitution.cpp
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
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(AnalysisMerge_Tests
  ${AnalysisMerge_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp 
  ${CORE_DIR}/Utils/AnalysisUtils.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  )
target_link_libraries(AnalysisMerge_Tests ${LIB_DEPENDECIES} )

add_test("\"Analysis Merge: Merge Analyses without classification\""   AnalysisMerge_Tests analysis_merge_merge_analyses_without_classification)
add_test("\"Analysis Merge: Merge Analyses with one classification\""  AnalysisMerge_Tests analysis_merge_merge_analyses_with_one_classification)
add_test("\"Analysis Merge: Merge Analyses with two classification\""  AnalysisMerge_Tests analysis_merge_merge_analyses_with_two_classifications)
add_test("\"Analysis Merge: Merge Analyses with category overlaping\"" AnalysisMerge_Tests analysis_merge_merge_analyses_with_category_overlaping)
add_test("\"Analysis Merge: Merge Analyses with same sample\""         AnalysisMerge_Tests analysis_merge_merge_analyses_with_same_sample)
add_test("\"Analysis Merge: Merge Analyses with same channel\""        AnalysisMerge_Tests analysis_merge_merge_analyses_with_same_channel)
add_test("\"Analysis Merge: Check segmentations input substitution\""  AnalysisMerge_Tests analysis_merge_merge_analyses_input_substitution)