# ChannelProxy tests
create_test_sourcelist(ChannelProxy_Tests ChannelProxy_Tests.cpp # this file is created by this command
  channel_proxy_add_channel.cpp
  channel_proxy_add_channels.cpp
  channel_proxy_add_existing_channel.cpp
  channel_proxy_add_existing_relation.cpp
  channel_proxy_add_existing_sample.cpp
  channel_proxy_add_existing_segmentation.cpp
  channel_proxy_add_relation.cpp
  channel_proxy_add_sample.cpp
  channel_proxy_add_samples.cpp
  channel_proxy_add_segmentation.cpp
  channel_proxy_add_segmentations.cpp
  channel_proxy_remove_sample.cpp
  channel_proxy_remove_samples.cpp
  channel_proxy_remove_channel.cpp
  channel_proxy_remove_channels.cpp
  channel_proxy_remove_segmentation.cpp
  channel_proxy_remove_segmentations.cpp
  channel_proxy_delete_relation.cpp
  channel_proxy_delete_non_existing_relation.cpp
  channel_proxy_remove_non_existing_sample.cpp
  channel_proxy_remove_non_existing_channel.cpp
  channel_proxy_remove_non_existing_segmentation.cpp
  channel_proxy_empty_analysis_constructor.cpp
  channel_proxy_replace_classification.cpp
  channel_proxy_set_classification.cpp
  channel_proxy_set_empty_classification.cpp
  channel_proxy_reset.cpp
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
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ModelAdapter.h
  ${GUI_DIR}/Model/FilterAdapter.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/Proxies/ChannelProxy.h
  ModelTest.h
)

add_executable(ChannelProxy_Tests
  ${ChannelProxy_Tests}
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
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp 
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${EXTENSIONS_DIR}/Notes/SegmentationNotes.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  ${GUI_DIR}/Model/ModelAdapter.cpp
  ${GUI_DIR}/Model/NeuroItemAdapter.cpp
  ${GUI_DIR}/Model/Proxies/ChannelProxy.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
  ${GUI_DIR}/Utils/Conditions.cpp
  ModelTest.cpp
  )
target_link_libraries(ChannelProxy_Tests ${LIB_DEPENDECIES} )

add_test("\"Channel Proxy: Empty Analysis Constructor\""              ChannelProxy_Tests channel_proxy_empty_analysis_constructor)
add_test("\"Channel Proxy: Add Sample\""                              ChannelProxy_Tests channel_proxy_add_sample)
add_test("\"Channel Proxy: Add Samples\""                             ChannelProxy_Tests channel_proxy_add_samples)
add_test("\"Channel Proxy: Add Channel\""                             ChannelProxy_Tests channel_proxy_add_channel)
add_test("\"Channel Proxy: Add Channels\""                            ChannelProxy_Tests channel_proxy_add_channels)
add_test("\"Channel Proxy: Add Segmentation\""                        ChannelProxy_Tests channel_proxy_add_segmentation)
add_test("\"Channel Proxy: Add Segmentations\""                       ChannelProxy_Tests channel_proxy_add_segmentations)
add_test("\"Channel Proxy: Add Relation\""                            ChannelProxy_Tests channel_proxy_add_relation)
add_test("\"Channel Proxy: Add Existing Sample\""                     ChannelProxy_Tests channel_proxy_add_existing_sample)
add_test("\"Channel Proxy: Add Existing Channel\""                    ChannelProxy_Tests channel_proxy_add_existing_channel)
add_test("\"Channel Proxy: Add Existing Segmentation\""               ChannelProxy_Tests channel_proxy_add_existing_segmentation)
add_test("\"Channel Proxy: Add Existing Relation\""                   ChannelProxy_Tests channel_proxy_add_existing_relation)
add_test("\"Channel Proxy: Remove Sample\""                           ChannelProxy_Tests channel_proxy_remove_sample)
add_test("\"Channel Proxy: Remove Samples\""                          ChannelProxy_Tests channel_proxy_remove_samples)
add_test("\"Channel Proxy: Remove Channel\""                          ChannelProxy_Tests channel_proxy_remove_channel)
add_test("\"Channel Proxy: Remove Channels\""                         ChannelProxy_Tests channel_proxy_remove_channels)
add_test("\"Channel Proxy: Remove Segmentation\""                     ChannelProxy_Tests channel_proxy_remove_segmentation)
add_test("\"Channel Proxy: Remove Segmentations\""                    ChannelProxy_Tests channel_proxy_remove_segmentations)
add_test("\"Channel Proxy: Delete Relation\""                         ChannelProxy_Tests channel_proxy_delete_relation)
add_test("\"Channel Proxy: Remove Non Existing Sample\""              ChannelProxy_Tests channel_proxy_remove_non_existing_sample)
add_test("\"Channel Proxy: Remove Non Existing Channel\""             ChannelProxy_Tests channel_proxy_remove_non_existing_channel)
add_test("\"Channel Proxy: Remove Non Existing Segmentation\""        ChannelProxy_Tests channel_proxy_remove_non_existing_segmentation)
add_test("\"Channel Proxy: Delete Non Existing Relation\""            ChannelProxy_Tests channel_proxy_delete_non_existing_relation)
add_test("\"Channel Proxy: Set Empty Classification\""                ChannelProxy_Tests channel_proxy_set_empty_classification)
add_test("\"Channel Proxy: Set Classification\""                      ChannelProxy_Tests channel_proxy_set_classification)
add_test("\"Channel Proxy: Replace Classification\""                  ChannelProxy_Tests channel_proxy_replace_classification)
add_test("\"Channel Proxy: Reset\""                                   ChannelProxy_Tests channel_proxy_reset)
