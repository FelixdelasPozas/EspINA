# Analysis tests
create_test_sourcelist(Analysis_Tests Analysis_Tests.cpp # this file is created by this command
  analysis_add_channel.cpp
  analysis_add_channels.cpp
  analysis_add_basic_pipeline.cpp
  analysis_add_complex_pipeline.cpp
  analysis_add_existing_channel.cpp
  analysis_add_existing_relation.cpp
  analysis_add_existing_sample.cpp
  analysis_add_existing_segmentation.cpp
  analysis_add_relation.cpp
  analysis_add_sample.cpp
  analysis_add_samples.cpp
  analysis_add_segmentation.cpp
  analysis_add_segmentations.cpp
  analysis_change_segmentation_output.cpp
  analysis_delete_non_existing_relation.cpp
  analysis_delete_relation.cpp
  analysis_remove_channel.cpp
  analysis_remove_channels.cpp
  analysis_remove_non_existing_channel.cpp
  analysis_remove_non_existing_sample.cpp
  analysis_remove_non_existing_segmentation.cpp
  analysis_remove_sample.cpp
  analysis_remove_samples.cpp
  analysis_remove_segmentation.cpp
  analysis_remove_segmentations.cpp
  analysis_remove_segmentation_from_basic_pipeline.cpp
  analysis_set_classification.cpp
  analysis_reset.cpp
)

add_executable(Analysis_Tests
  analysis_testing_support.cpp
  ${Analysis_Tests}
  )

target_link_libraries(Analysis_Tests ${CORE_DEPENDECIES} )

add_test("\"Analysis: Add Sample\""                              Analysis_Tests analysis_add_sample)
add_test("\"Analysis: Add Samples\""                             Analysis_Tests analysis_add_samples)
add_test("\"Analysis: Add Channel\""                             Analysis_Tests analysis_add_channel)
add_test("\"Analysis: Add Channels\""                            Analysis_Tests analysis_add_channels)
add_test("\"Analysis: Add Segmentation\""                        Analysis_Tests analysis_add_segmentation)
add_test("\"Analysis: Add Segmentations\""                       Analysis_Tests analysis_add_segmentations)
add_test("\"Analysis: Add Basic Pipeline\""                      Analysis_Tests analysis_add_basic_pipeline)
add_test("\"Analysis: Add Complex Pipeline\""                    Analysis_Tests analysis_add_complex_pipeline)
add_test("\"Analysis: Change Segmentation Output\""              Analysis_Tests analysis_change_segmentation_output)
add_test("\"Analysis: Add Relation\""                            Analysis_Tests analysis_add_relation)
add_test("\"Analysis: Add Relation Between Non Existing Items\"" Analysis_Tests analysis_add_relation)
add_test("\"Analysis: Add Existing Sample\""                     Analysis_Tests analysis_add_existing_sample)
add_test("\"Analysis: Add Existing Channel\""                    Analysis_Tests analysis_add_existing_channel)
add_test("\"Analysis: Add Existing Segmentation\""               Analysis_Tests analysis_add_existing_segmentation)
add_test("\"Analysis: Add Existing Relation\""                   Analysis_Tests analysis_add_existing_relation)
add_test("\"Analysis: Remove Sample\""                           Analysis_Tests analysis_remove_sample)
add_test("\"Analysis: Remove Samples\""                          Analysis_Tests analysis_remove_samples)
add_test("\"Analysis: Remove Channel\""                          Analysis_Tests analysis_remove_channel)
add_test("\"Analysis: Remove Channels\""                         Analysis_Tests analysis_remove_channels)
add_test("\"Analysis: Remove Segmentation\""                     Analysis_Tests analysis_remove_segmentation)
add_test("\"Analysis: Remove Segmentations\""                    Analysis_Tests analysis_remove_segmentations)
add_test("\"Analysis: Remove Segmentation from Basic Pipeline\"" Analysis_Tests analysis_remove_segmentation_from_basic_pipeline)
add_test("\"Analysis: Delete Relation\""                         Analysis_Tests analysis_delete_relation)
add_test("\"Analysis: Remove Non Existing Sample\""              Analysis_Tests analysis_remove_non_existing_sample)
add_test("\"Analysis: Remove Non Existing Channel\""             Analysis_Tests analysis_remove_non_existing_channel)
add_test("\"Analysis: Remove Non Existing Segmentation\""        Analysis_Tests analysis_remove_non_existing_segmentation)
add_test("\"Analysis: Delete Non Existing Relation\""            Analysis_Tests analysis_delete_non_existing_relation)
add_test("\"Analysis: Set Classification\""                      Analysis_Tests analysis_set_classification)
add_test("\"Analysis: Reset\""                                   Analysis_Tests analysis_reset)
