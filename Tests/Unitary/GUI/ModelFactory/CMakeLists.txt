# Model Factory Tests
create_test_sourcelist(TEST_SOURCES Model_Factory_Tests.cpp # this file is created by this command
  model_factory_create_filter.cpp
)

add_executable(Model_Factory_Tests "" ${TEST_SOURCES} )

target_link_libraries(Model_Factory_Tests ${GUI_DEPENDECIES} )

add_test("\"Model Factory: Create Filter\"" Model_Factory_Tests model_factory_create_filter)
