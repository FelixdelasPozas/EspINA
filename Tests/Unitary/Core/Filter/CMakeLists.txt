# Bound tests
create_test_sourcelist(TEST_SOURCES Filter_Tests.cpp # this file is created by this command
  filter_output_update.cpp
  filter_update.cpp
)


add_executable(Filter_Tests "" ${TEST_SOURCES})

target_link_libraries(Filter_Tests ${CORE_DEPENDECIES} )

add_test("\"Filter: Output Update\"" Filter_Tests filter_output_update)
add_test("\"Filter: Update\"" Filter_Tests filter_update)