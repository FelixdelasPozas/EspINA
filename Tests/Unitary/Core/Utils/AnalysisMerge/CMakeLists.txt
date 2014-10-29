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


add_executable(AnalysisMerge_Tests "" ${AnalysisMerge_Tests} )
target_link_libraries(AnalysisMerge_Tests ${CORE_DEPENDECIES} )

add_test("\"Analysis Merge: Merge Analyses without classification\""   AnalysisMerge_Tests analysis_merge_merge_analyses_without_classification)
add_test("\"Analysis Merge: Merge Analyses with one classification\""  AnalysisMerge_Tests analysis_merge_merge_analyses_with_one_classification)
add_test("\"Analysis Merge: Merge Analyses with two classification\""  AnalysisMerge_Tests analysis_merge_merge_analyses_with_two_classifications)
add_test("\"Analysis Merge: Merge Analyses with category overlaping\"" AnalysisMerge_Tests analysis_merge_merge_analyses_with_category_overlaping)
add_test("\"Analysis Merge: Merge Analyses with same sample\""         AnalysisMerge_Tests analysis_merge_merge_analyses_with_same_sample)
add_test("\"Analysis Merge: Merge Analyses with same channel\""        AnalysisMerge_Tests analysis_merge_merge_analyses_with_same_channel)
add_test("\"Analysis Merge: Check segmentations input substitution\""  AnalysisMerge_Tests analysis_merge_merge_analyses_input_substitution)