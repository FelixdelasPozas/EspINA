# ChannelProxy tests
create_test_sourcelist(ChannelProxy_Tests ChannelProxy_Tests.cpp # this file is created by this command
  channel_proxy_add_channel.cpp
  channel_proxy_add_channels.cpp
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
  channel_proxy_set_classification.cpp
  channel_proxy_reset.cpp
)

if (UNIX)
  set (LIB_DEPENDECIES
     ${GUI_DEPENDECIES}
     QtTest
    )
endif (UNIX)

add_executable(ChannelProxy_Tests "" ${ChannelProxy_Tests} ${SUBJECT_MOCS})
target_link_libraries(ChannelProxy_Tests ${LIB_DEPENDECIES} )

add_test("\"Channel Proxy: Empty Analysis Constructor\""              ChannelProxy_Tests channel_proxy_empty_analysis_constructor)
add_test("\"Channel Proxy: Add Sample\""                              ChannelProxy_Tests channel_proxy_add_sample)
add_test("\"Channel Proxy: Add Samples\""                             ChannelProxy_Tests channel_proxy_add_samples)
add_test("\"Channel Proxy: Add Channel\""                             ChannelProxy_Tests channel_proxy_add_channel)
add_test("\"Channel Proxy: Add Channels\""                            ChannelProxy_Tests channel_proxy_add_channels)
add_test("\"Channel Proxy: Add Segmentation\""                        ChannelProxy_Tests channel_proxy_add_segmentation)
add_test("\"Channel Proxy: Add Segmentations\""                       ChannelProxy_Tests channel_proxy_add_segmentations)
add_test("\"Channel Proxy: Add Relation\""                            ChannelProxy_Tests channel_proxy_add_relation)
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
add_test("\"Channel Proxy: Set Classification\""                      ChannelProxy_Tests channel_proxy_set_classification)
add_test("\"Channel Proxy: Reset\""                                   ChannelProxy_Tests channel_proxy_reset)
