# Seed Grow Segmentation Filter Tests
create_test_sourcelist(TEST_SOURCES VolumetricStreamReader_Tests.cpp # this file is created by this command
  volumetric_stream_reader_simple_execution.cpp
)

add_executable(VolumetricStreamReader_Tests "" ${TEST_SOURCES} )

target_link_libraries(VolumetricStreamReader_Tests ${FILTERS_DEPENDECIES} )

add_test("\"Volumetric Stream Reader: Simple Execution\"" VolumetricStreamReader_Tests volumetric_stream_reader_simple_execution)