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

add_executable(ChannelAdapter_Tests ${TEST_SOURCES} ../gui_testing_support.cpp)

target_link_libraries(ChannelAdapter_Tests ${GUI_DEPENDECIES} )

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
