# Core Factory Tests
create_test_sourcelist(TEST_SOURCES Core_Factory_Tests.cpp # this file is created by this command
  core_factory_register_filter_factory.cpp
)

add_executable(Core_Factory_Tests
  ${TEST_SOURCES}
  )

target_link_libraries(Core_Factory_Tests ${CORE_DEPENDECIES} )

add_test("\"Core Factory: Register Filter Factory\""  Core_Factory_Tests core_factory_register_filter_factory)