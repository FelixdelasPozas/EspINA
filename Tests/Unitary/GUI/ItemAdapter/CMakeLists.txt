# Item Adapter Interface Tests
create_test_sourcelist(TEST_SOURCES ItemAdapter_Tests.cpp # this file is created by this command
  item_adapter_interface.cpp
)

include_directories(
#   ${SUBJECT_DIR}
 ${CMAKE_BINARY_DIR}/GUI
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${GUI_DIR}/Model/ItemAdapter.h
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
#   ${VTK_LIBRARIES}
#   ${ITK_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${LIB_DEPENDECIES}
)

add_executable(ItemAdapter_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
#   ${CORE_DIR}/Utils/Bounds.cpp
#   ${CORE_DIR}/Analysis/Channel.cpp
#   ${CORE_DIR}/Analysis/Filter.cpp
#   ${CORE_DIR}/Analysis/Output.cpp
#   ${CORE_DIR}/Analysis/Data.cpp
#   ${CORE_DIR}/Analysis/ViewItem.cpp
#   ${CORE_DIR}/MultiTasking/Scheduler.cpp
#   ${CORE_DIR}/MultiTasking/Task.cpp
#   ${GUI_DIR}/ModelFactory.cpp
#   ${GUI_DIR}/Model/ChannelAdapter.cpp
  )

target_link_libraries(ItemAdapter_Tests ${LIB_DEPENDECIES} )

add_test("\"Item Adapter: Interface\"" ItemAdapter_Tests item_adapter_interface)