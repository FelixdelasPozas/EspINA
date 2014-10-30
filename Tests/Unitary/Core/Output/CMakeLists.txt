# Bound tests
create_test_sourcelist(TEST_SOURCES Output_Tests.cpp # this file is created by this command
  output_valid_output.cpp
  output_invalid_output.cpp
  output_update_filter.cpp
  output_replace_data.cpp
  output_remove_data.cpp
)

add_executable(Output_Tests "" ${TEST_SOURCES})

target_link_libraries(Output_Tests ${CORE_DEPENDECIES} )

add_test("\"Output: Create Invalid Output\"" Output_Tests output_invalid_output)
add_test("\"Output: Create Valid Output\""   Output_Tests output_valid_output)
add_test("\"Output: Update Filter\""         Output_Tests output_update_filter)
add_test("\"Output: Replace Data\""          Output_Tests output_replace_data)
add_test("\"Output: Remove Data\""           Output_Tests output_remove_data)