# Channel Tests
create_test_sourcelist(Channel_Tests Channel_Tests.cpp # this file is created by this command
  channel_add_extension.cpp
  channel_remove_extension.cpp
  channel_set_hue.cpp
  channel_set_saturation.cpp
  channel_set_opacity.cpp
  channel_set_contrast.cpp
  channel_set_brightness.cpp
  channel_set_invalid_hue.cpp
  channel_set_invalid_saturation.cpp
  channel_set_invalid_opacity.cpp
  channel_set_invalid_contrast.cpp
  channel_set_invalid_brightness.cpp
)

add_executable(Channel_Tests "" ${Channel_Tests} ) #"" is a hack to display target on kdevelop

target_link_libraries(Channel_Tests ${CORE_DEPENDECIES} )

add_test("\"Channel: Set Hue\""                Channel_Tests channel_set_hue)
add_test("\"Channel: Set Saturation\""         Channel_Tests channel_set_saturation)
add_test("\"Channel: Set Opacity\""            Channel_Tests channel_set_opacity)
add_test("\"Channel: Set Contrast\""           Channel_Tests channel_set_contrast)
add_test("\"Channel: Set Brightness\""         Channel_Tests channel_set_brightness)
add_test("\"Channel: Set Invalid Hue\""        Channel_Tests channel_set_invalid_hue)
add_test("\"Channel: Set Invalid Saturation\"" Channel_Tests channel_set_invalid_saturation)
add_test("\"Channel: Set Invalid Opacity\""    Channel_Tests channel_set_invalid_opacity)
add_test("\"Channel: Set Invalid Contrast\""   Channel_Tests channel_set_invalid_contrast)
add_test("\"Channel: Set Invalid Brightness\"" Channel_Tests channel_set_invalid_brightness)
#add_test("\"Channel: Change Bounds\""          Channel_Tests channel_)
#add_test("\"Channel: Set Position\""           Channel_Tests channel_set)
add_test("\"Channel: Add Extension\""          Channel_Tests channel_add_extension)
#add_test("\"Channel: Add Dependent Extension\""          Channel_Tests channel_add_extension)
add_test("\"Channel: Remove Extension\""       Channel_Tests channel_remove_extension)
#add_test("\"Channel: Delete Extension Dependency\""          Channel_Tests channel_add_extension)
# add_test("\"Channel: Save State\""             Channel_Tests channel_)
# add_test("\"Channel: Restore State\""          Channel_Tests channel_)
# add_test("\"Channel: Save Snapshot\""          Channel_Tests channel_)
