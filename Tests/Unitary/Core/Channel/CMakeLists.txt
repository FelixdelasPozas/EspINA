# Channel Tests
create_test_sourcelist(Channel_Tests Channel_Tests.cpp # this file is created by this command
  channel_add_extension.cpp
  channel_delete_extension.cpp
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

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(Channel_Tests
  ${Channel_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  )
target_link_libraries(Channel_Tests ${LIB_DEPENDECIES} )

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
add_test("\"Channel: Delete Extension\""       Channel_Tests channel_delete_extension)
#add_test("\"Channel: Delete Extension Dependency\""          Channel_Tests channel_add_extension)
# add_test("\"Channel: Save State\""             Channel_Tests channel_)
# add_test("\"Channel: Restore State\""          Channel_Tests channel_)
# add_test("\"Channel: Save Snapshot\""          Channel_Tests channel_)
