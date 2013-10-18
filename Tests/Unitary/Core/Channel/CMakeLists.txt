# Bound tests
create_test_sourcelist(TEST_SOURCES Channel_Tests.cpp # this file is created by this command
  channel_add_sample.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")

include_directories(
  ${SUBJECT_DIR}
  )

# qt4_wrap_cpp(SUBJECT_MOCS
#   ${SUBJECT_DIR}/Category.h
#   ${SUBJECT_DIR}/Data.h
# )

# set(SUBJECT_SOURCES
#   ${SUBJECT_DIR}/Classification.cpp
#   ${SUBJECT_DIR}/Category.cpp
# )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Channel_Tests
  ${TEST_SOURCES}
#   ${SUBJECT_MOCS}
#   ${SUBJECT_SOURCES}
  )

target_link_libraries(Channel_Tests ${LIB_DEPENDECIES})

add_test("\"Channel: Default Constructor\"" Channel_Tests channel_default_constructor)
add_test("\"Channel: Set Hue\"" Channel_Tests channel_change_hue)
add_test("\"Channel: Set Saturation\"" Channel_Tests channel_)
add_test("\"Channel: Set Opacity\"" Channel_Tests channel_)
add_test("\"Channel: Set Contrast\"" Channel_Tests channel_)
add_test("\"Channel: Set Brightness\"" Channel_Tests channel_)
add_test("\"Channel: Set Position\"" Channel_Tests channel_)
add_test("\"Channel: Set Invalid Hue\"" Channel_Tests channel_change_hue)
add_test("\"Channel: Set Invalid Saturation\"" Channel_Tests channel_)
add_test("\"Channel: Set Invalid Opacity\"" Channel_Tests channel_)
add_test("\"Channel: Set Invalid Contrast\"" Channel_Tests channel_)
add_test("\"Channel: Set Invalid Brightness\"" Channel_Tests channel_)
#add_test("\"Channel: Change Bounds\"" Channel_Tests channel_)
add_test("\"Channel: Add Extension\"" Channel_Tests channel_)
add_test("\"Channel: Delete Extension\"" Channel_Tests channel_)
add_test("\"Channel: Save State\"" Channel_Tests channel_)
add_test("\"Channel: Restore State\"" Channel_Tests channel_)
add_test("\"Channel: Save Snapshot\"" Channel_Tests channel_)