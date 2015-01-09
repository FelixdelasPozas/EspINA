# ModelAdapter tests
create_test_sourcelist(ModelAdapter_Tests ModelAdapter_Tests.cpp # this file is created by this command
  model_adapter_add_channel.cpp
  model_adapter_add_channels.cpp
  model_adapter_add_existing_channel.cpp
  model_adapter_add_existing_relation.cpp
  model_adapter_add_existing_sample.cpp
  model_adapter_add_existing_segmentation.cpp
  model_adapter_add_relation.cpp
  model_adapter_add_sample.cpp
  model_adapter_add_samples.cpp
  model_adapter_add_segmentation.cpp
  model_adapter_add_segmentations.cpp
  model_adapter_remove_sample.cpp
  model_adapter_remove_samples.cpp
  model_adapter_remove_channel.cpp
  model_adapter_remove_channels.cpp
  model_adapter_remove_segmentation.cpp
  model_adapter_remove_segmentations.cpp
  model_adapter_delete_relation.cpp
  model_adapter_delete_non_existing_relation.cpp
  model_adapter_remove_non_existing_sample.cpp
  model_adapter_remove_non_existing_channel.cpp
  model_adapter_remove_non_existing_segmentation.cpp
  model_adapter_empty_analysis_constructor.cpp
  model_adapter_replace_classification.cpp
  model_adapter_set_classification.cpp
  model_adapter_set_empty_classification.cpp
  model_adapter_clear.cpp
)

add_executable(ModelAdapter_Tests "" ${ModelAdapter_Tests})
target_link_libraries(ModelAdapter_Tests ${GUI_DEPENDECIES} ${TESTING_DEPENDECIES})

add_test("\"Model Adapter: Empty Analysis Constructor\""              ModelAdapter_Tests model_adapter_empty_analysis_constructor)
add_test("\"Model Adapter: Add Sample\""                              ModelAdapter_Tests model_adapter_add_sample)
add_test("\"Model Adapter: Add Samples\""                             ModelAdapter_Tests model_adapter_add_samples)
add_test("\"Model Adapter: Add Channel\""                             ModelAdapter_Tests model_adapter_add_channel)
add_test("\"Model Adapter: Add Channels\""                            ModelAdapter_Tests model_adapter_add_channels)
add_test("\"Model Adapter: Add Segmentation\""                        ModelAdapter_Tests model_adapter_add_segmentation)
add_test("\"Model Adapter: Add Segmentations\""                       ModelAdapter_Tests model_adapter_add_segmentations)
add_test("\"Model Adapter: Add Relation\""                            ModelAdapter_Tests model_adapter_add_relation)
# add_test("\"Model Adapter: Add Relation Between Non Existing Items\"" ModelAdapter_Tests model_adapter_add_relation)
add_test("\"Model Adapter: Add Existing Sample\""                     ModelAdapter_Tests model_adapter_add_existing_sample)
add_test("\"Model Adapter: Add Existing Channel\""                    ModelAdapter_Tests model_adapter_add_existing_channel)
add_test("\"Model Adapter: Add Existing Segmentation\""               ModelAdapter_Tests model_adapter_add_existing_segmentation)
add_test("\"Model Adapter: Add Existing Relation\""                   ModelAdapter_Tests model_adapter_add_existing_relation)
add_test("\"Model Adapter: Remove Sample\""                           ModelAdapter_Tests model_adapter_remove_sample)
add_test("\"Model Adapter: Remove Samples\""                          ModelAdapter_Tests model_adapter_remove_samples)
add_test("\"Model Adapter: Remove Channel\""                          ModelAdapter_Tests model_adapter_remove_channel)
add_test("\"Model Adapter: Remove Channels\""                         ModelAdapter_Tests model_adapter_remove_channels)
add_test("\"Model Adapter: Remove Segmentation\""                     ModelAdapter_Tests model_adapter_remove_segmentation)
add_test("\"Model Adapter: Remove Segmentations\""                    ModelAdapter_Tests model_adapter_remove_segmentations)
add_test("\"Model Adapter: Delete Relation\""                         ModelAdapter_Tests model_adapter_delete_relation)
add_test("\"Model Adapter: Remove Non Existing Sample\""              ModelAdapter_Tests model_adapter_remove_non_existing_sample)
add_test("\"Model Adapter: Remove Non Existing Channel\""             ModelAdapter_Tests model_adapter_remove_non_existing_channel)
add_test("\"Model Adapter: Remove Non Existing Segmentation\""        ModelAdapter_Tests model_adapter_remove_non_existing_segmentation)
add_test("\"Model Adapter: Delete Non Existing Relation\""            ModelAdapter_Tests model_adapter_delete_non_existing_relation)
add_test("\"Model Adapter: Set Empty Classification\""                ModelAdapter_Tests model_adapter_set_empty_classification)
add_test("\"Model Adapter: Set Classification\""                      ModelAdapter_Tests model_adapter_set_classification)
add_test("\"Model Adapter: Replace Classification\""                  ModelAdapter_Tests model_adapter_replace_classification)
add_test("\"Model Adapter: Clear\""                                   ModelAdapter_Tests model_adapter_clear)
