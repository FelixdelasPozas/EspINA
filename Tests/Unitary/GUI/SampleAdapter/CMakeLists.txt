create_test_sourcelist(TEST_SOURCES SampleAdapter_Tests.cpp # this file is created by this command
  sample_adapter_set_name.cpp
  sample_adapter_set_bounds.cpp
  sample_adapter_set_position.cpp
  sample_adapter_model_data.cpp
)

qt4_wrap_cpp(TEST_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/FilterAdapter.h
  ${GUI_DIR}/Model/ItemAdapter.h
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(SampleAdapter_Tests
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
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
)

target_link_libraries(SampleAdapter_Tests ${LIB_DEPENDECIES} )

add_test("\"Sample Adapter: Set Name\""      SampleAdapter_Tests sample_adapter_set_name)
add_test("\"Sample Adapter: Set Bounds\""    SampleAdapter_Tests sample_adapter_set_bounds)
add_test("\"Sample Adapter: Set Position\""  SampleAdapter_Tests sample_adapter_set_position)
add_test("\"Sample Adapter: Model Data\""    SampleAdapter_Tests sample_adapter_model_data)
