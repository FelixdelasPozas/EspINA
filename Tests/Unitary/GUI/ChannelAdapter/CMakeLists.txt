create_test_sourcelist(TEST_SOURCES ChannelAdapter_Tests.cpp # this file is created by this command
  channel_adapter_set_hue.cpp
  channel_adapter_set_invalid_hue.cpp
  channel_adapter_set_saturation.cpp
  channel_adapter_set_invalid_saturation.cpp
  channel_adapter_set_opacity.cpp
  channel_adapter_set_invalid_opacity.cpp
  channel_adapter_set_contrast.cpp
  channel_adapter_set_invalid_contrast.cpp
  channel_adapter_set_brightness.cpp
  channel_adapter_set_invalid_brightness.cpp
)

qt4_wrap_cpp(TEST_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/OutputAdapter.h
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(ChannelAdapter_Tests
  ${TEST_SOURCES}
  ${TEST_MOCS}
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/OutputAdapter.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
  ../gui_testing_support.cpp
)

target_link_libraries(ChannelAdapter_Tests ${LIB_DEPENDECIES} )

add_test("\"Channel Adapter: Set Hue\""                ChannelAdapter_Tests channel_adapter_set_hue)
add_test("\"Channel Adapter: Set Saturation\""         ChannelAdapter_Tests channel_adapter_set_saturation)
add_test("\"Channel Adapter: Set Opacity\""            ChannelAdapter_Tests channel_adapter_set_opacity)
add_test("\"Channel Adapter: Set Contrast\""           ChannelAdapter_Tests channel_adapter_set_contrast)
add_test("\"Channel Adapter: Set Brightness\""         ChannelAdapter_Tests channel_adapter_set_brightness)
add_test("\"Channel Adapter: Set Invalid Hue\""        ChannelAdapter_Tests channel_adapter_set_invalid_hue)
add_test("\"Channel Adapter: Set Invalid Saturation\"" ChannelAdapter_Tests channel_adapter_set_invalid_saturation)
add_test("\"Channel Adapter: Set Invalid Opacity\""    ChannelAdapter_Tests channel_adapter_set_invalid_opacity)
add_test("\"Channel Adapter: Set Invalid Contrast\""   ChannelAdapter_Tests channel_adapter_set_invalid_contrast)
add_test("\"Channel Adapter: Set Invalid Brightness\"" ChannelAdapter_Tests channel_adapter_set_invalid_brightness)