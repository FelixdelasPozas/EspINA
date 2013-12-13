# ClassificationProxy tests
create_test_sourcelist(ClassificationProxy_Tests ClassificationProxy_Tests.cpp # this file is created by this command
#   classification_proxy_add_channel.cpp
#   classification_proxy_add_channels.cpp
#   classification_proxy_add_existing_channel.cpp
#   classification_proxy_add_existing_relation.cpp
#   classification_proxy_add_existing_sample.cpp
#   classification_proxy_add_existing_segmentation.cpp
#   classification_proxy_add_relation.cpp
#   classification_proxy_add_sample.cpp
#   classification_proxy_add_samples.cpp
  classification_proxy_add_segmentation.cpp
#   classification_proxy_add_segmentations.cpp
classification_proxy_add_segmentation_without_category.cpp
#   classification_proxy_remove_sample.cpp
#   classification_proxy_remove_samples.cpp
#   classification_proxy_remove_channel.cpp
#   classification_proxy_remove_channels.cpp
#   classification_proxy_remove_segmentation.cpp
#   classification_proxy_remove_segmentations.cpp
#   classification_proxy_delete_relation.cpp
#   classification_proxy_delete_non_existing_relation.cpp
#   classification_proxy_remove_non_existing_sample.cpp
#   classification_proxy_remove_non_existing_channel.cpp
#   classification_proxy_remove_non_existing_segmentation.cpp
  classification_proxy_empty_analysis_constructor.cpp
#   classification_proxy_replace_classification.cpp
  classification_proxy_set_classification.cpp
#   classification_proxy_set_empty_classification.cpp
#   classification_proxy_reset.cpp
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
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ModelAdapter.h
  ${GUI_DIR}/Model/FilterAdapter.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.h
  ModelTest.h
)

add_executable(ClassificationProxy_Tests
  ${ClassificationProxy_Tests}
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
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp 
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  ${GUI_DIR}/Model/ModelAdapter.cpp
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
  ModelTest.cpp
  )
target_link_libraries(ClassificationProxy_Tests ${LIB_DEPENDECIES} )

add_test("\"Classification Proxy: Empty Analysis Constructor\""              ClassificationProxy_Tests classification_proxy_empty_analysis_constructor)
# add_test("\"Classification Proxy: Add Sample\""                              ClassificationProxy_Tests classification_proxy_add_sample)
# add_test("\"Classification Proxy: Add Samples\""                             ClassificationProxy_Tests classification_proxy_add_samples)
# add_test("\"Classification Proxy: Add Channel\""                             ClassificationProxy_Tests classification_proxy_add_channel)
# add_test("\"Classification Proxy: Add Channels\""                            ClassificationProxy_Tests classification_proxy_add_channels)
add_test("\"Classification Proxy: Add Segmentation\""                        ClassificationProxy_Tests classification_proxy_add_segmentation)
# add_test("\"Classification Proxy: Add Segmentations\""                       ClassificationProxy_Tests classification_proxy_add_segmentations)
add_test("\"Classification Proxy: Add Segmentation Without Category\""       ClassificationProxy_Tests classification_proxy_add_segmentation_without_category)
# add_test("\"Classification Proxy: Add Relation\""                            ClassificationProxy_Tests classification_proxy_add_relation)
# add_test("\"Classification Proxy: Add Existing Sample\""                     ClassificationProxy_Tests classification_proxy_add_existing_sample)
# add_test("\"Classification Proxy: Add Existing Channel\""                    ClassificationProxy_Tests classification_proxy_add_existing_channel)
# add_test("\"Classification Proxy: Add Existing Segmentation\""               ClassificationProxy_Tests classification_proxy_add_existing_segmentation)
# add_test("\"Classification Proxy: Add Existing Relation\""                   ClassificationProxy_Tests classification_proxy_add_existing_relation)
# add_test("\"Classification Proxy: Remove Sample\""                           ClassificationProxy_Tests classification_proxy_remove_sample)
# add_test("\"Classification Proxy: Remove Samples\""                          ClassificationProxy_Tests classification_proxy_remove_samples)
# add_test("\"Classification Proxy: Remove Channel\""                          ClassificationProxy_Tests classification_proxy_remove_channel)
# add_test("\"Classification Proxy: Remove Channels\""                         ClassificationProxy_Tests classification_proxy_remove_channels)
# add_test("\"Classification Proxy: Remove Segmentation\""                     ClassificationProxy_Tests classification_proxy_remove_segmentation)
# add_test("\"Classification Proxy: Remove Segmentations\""                    ClassificationProxy_Tests classification_proxy_remove_segmentations)
# add_test("\"Classification Proxy: Delete Relation\""                         ClassificationProxy_Tests classification_proxy_delete_relation)
# add_test("\"Classification Proxy: Remove Non Existing Sample\""              ClassificationProxy_Tests classification_proxy_remove_non_existing_sample)
# add_test("\"Classification Proxy: Remove Non Existing Channel\""             ClassificationProxy_Tests classification_proxy_remove_non_existing_channel)
# add_test("\"Classification Proxy: Remove Non Existing Segmentation\""        ClassificationProxy_Tests classification_proxy_remove_non_existing_segmentation)
# add_test("\"Classification Proxy: Delete Non Existing Relation\""            ClassificationProxy_Tests classification_proxy_delete_non_existing_relation)
# add_test("\"Classification Proxy: Set Empty Classification\""                ClassificationProxy_Tests classification_proxy_set_empty_classification)
add_test("\"Classification Proxy: Set Classification\""                      ClassificationProxy_Tests classification_proxy_set_classification)
# add_test("\"Classification Proxy: Replace Classification\""                  ClassificationProxy_Tests classification_proxy_replace_classification)
# add_test("\"Classification Proxy: Reset\""                                   ClassificationProxy_Tests classification_proxy_reset)
