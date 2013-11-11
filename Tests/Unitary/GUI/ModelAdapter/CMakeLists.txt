# ModelAdapter tests
create_test_sourcelist(ModelAdapter_Tests ModelAdapter_Tests.cpp # this file is created by this command
  model_adapter_add_sample.cpp
  model_adapter_add_samples.cpp
  model_adapter_empty_analysis_constructor.cpp
  model_adapter_add_channel.cpp
  model_adapter_add_channels.cpp
  model_adapter_set_classification.cpp
  model_adapter_set_empty_classification.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

if (UNIX)
  set (LIB_DEPENDECIES
     ${LIB_DEPENDECIES}
     QtTest
    )
endif (UNIX)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ModelAdapter.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/OutputAdapter.h
  ModelTest.h
)

add_executable(ModelAdapter_Tests
  ${ModelAdapter_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp 
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  ${GUI_DIR}/Model/ModelAdapter.cpp
  ${GUI_DIR}/Model/OutputAdapter.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ModelTest.cpp
  )
target_link_libraries(ModelAdapter_Tests ${LIB_DEPENDECIES} )

add_test("\"Model Adapter: Empty Analysis Constructor\""              ModelAdapter_Tests model_adapter_empty_analysis_constructor)
add_test("\"Model Adapter: Add Sample\""                              ModelAdapter_Tests model_adapter_add_sample)
add_test("\"Model Adapter: Add Samples\""                             ModelAdapter_Tests model_adapter_add_samples)
add_test("\"Model Adapter: Add Channel\""                             ModelAdapter_Tests model_adapter_add_channel)
add_test("\"Model Adapter: Add Channels\""                            ModelAdapter_Tests model_adapter_add_channels)
# add_test("\"Model Adapter: Add Segmentation\""                        ModelAdapter_Tests model_adapter_add_segmentation)
# add_test("\"Model Adapter: Add Segmentations\""                       ModelAdapter_Tests model_adapter_add_segmentations)
# add_test("\"Model Adapter: Add Extension Provider\""                  ModelAdapter_Tests model_adapter_add_extension_provider)
# add_test("\"Model Adapter: Add Relation\""                            ModelAdapter_Tests model_adapter_add_relation)
# add_test("\"Model Adapter: Add Relation Between Non Existing Items\"" ModelAdapter_Tests model_adapter_add_relation)
# add_test("\"Model Adapter: Add Existing Sample\""                     ModelAdapter_Tests model_adapter_add_existing_sample)
# add_test("\"Model Adapter: Add Existing Channel\""                    ModelAdapter_Tests model_adapter_add_existing_channel)
# add_test("\"Model Adapter: Add Existing Segmentation\""               ModelAdapter_Tests model_adapter_add_existing_segmentation)
# add_test("\"Model Adapter: Add Existing Extension Provider\""         ModelAdapter_Tests model_adapter_add_existing_extension_provider)
# add_test("\"Model Adapter: Add Existing Relation\""                   ModelAdapter_Tests model_adapter_add_existing_relation)
# add_test("\"Model Adapter: Remove Sample\""                           ModelAdapter_Tests model_adapter_remove_sample)
# add_test("\"Model Adapter: Remove Samples\""                          ModelAdapter_Tests model_adapter_remove_samples)
# add_test("\"Model Adapter: Remove Channel\""                          ModelAdapter_Tests model_adapter_remove_channel)
# add_test("\"Model Adapter: Remove Channels\""                         ModelAdapter_Tests model_adapter_remove_channels)
# add_test("\"Model Adapter: Remove Segmentation\""                     ModelAdapter_Tests model_adapter_remove_segmentation)
# add_test("\"Model Adapter: Remove Segmentations\""                    ModelAdapter_Tests model_adapter_remove_segmentations)
# add_test("\"Model Adapter: Remove Extension Provider\""               ModelAdapter_Tests model_adapter_remove_extension_provider)
# add_test("\"Model Adapter: Delete Relation\""                         ModelAdapter_Tests model_adapter_delete_relation)
# add_test("\"Model Adapter: Remove Non Existing Sample\""              ModelAdapter_Tests model_adapter_remove_non_existing_sample)
# add_test("\"Model Adapter: Remove Non Existing Channel\""             ModelAdapter_Tests model_adapter_remove_non_existing_channel)
# add_test("\"Model Adapter: Remove Non Existing Segmentation\""        ModelAdapter_Tests model_adapter_remove_non_existing_segmentation)
# add_test("\"Model Adapter: Remove Non Existing Extension Provider\""  ModelAdapter_Tests model_adapter_remove_non_existing_extension_provider)
# add_test("\"Model Adapter: Delete Non Existing Relation\""            ModelAdapter_Tests model_adapter_delete_non_existing_relation)
add_test("\"Model Adapter: Set Empty Classification\""                ModelAdapter_Tests model_adapter_set_empty_classification)
add_test("\"Model Adapter: Set Classification\""                      ModelAdapter_Tests model_adapter_set_classification)
# add_test("\"Model Adapter: Reset\""                                   ModelAdapter_Tests model_adapter_reset)
# 