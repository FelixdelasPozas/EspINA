create_test_sourcelist(TEST_SOURCES SampleAdapter_Tests.cpp # this file is created by this command
  sample_adapter_set_name.cpp
  sample_adapter_set_bounds.cpp
  sample_adapter_set_position.cpp
  sample_adapter_model_data.cpp
)

add_executable(SampleAdapter_Tests "" ${TEST_SOURCES} )

target_link_libraries(SampleAdapter_Tests ${GUI_DEPENDECIES} )

add_test("\"Sample Adapter: Set Name\""      SampleAdapter_Tests sample_adapter_set_name)
add_test("\"Sample Adapter: Set Bounds\""    SampleAdapter_Tests sample_adapter_set_bounds)
add_test("\"Sample Adapter: Set Position\""  SampleAdapter_Tests sample_adapter_set_position)
add_test("\"Sample Adapter: Model Data\""    SampleAdapter_Tests sample_adapter_model_data)
