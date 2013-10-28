# Model Factory Tests
create_test_sourcelist(TEST_SOURCES Model_Factory_Tests.cpp # this file is created by this command
  model_factory_create_filter.cpp
  model_factory_create_template_filter.cpp
)

# include_directories(
#   ${SUBJECT_DIR}
#   ${CMAKE_CURRENT_SOURCE_DIR}
#   )
qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ItemAdapter.h
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${LIB_DEPENDECIES}
)

add_executable(Model_Factory_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  )

target_link_libraries(Model_Factory_Tests ${LIB_DEPENDECIES} )

add_test("\"Model Factory: Create Filter\""          Model_Factory_Tests model_factory_create_filter)
add_test("\"Model Factory: Create Template Filter\"" Model_Factory_Tests model_factory_create_template_filter)