
# Analysis tests
create_test_sourcelist(IO_Tests IO_Tests.cpp # this file is created by this command
  io_load_default_classification.cpp
  io_analysis_seg_file_io.cpp
  io_load_seg_file_analysis_without_registered_filters.cpp
  io_classification_xml_io.cpp
  io_sgs_sas.cpp
#   io_skeleton.cpp
  io_save_merged_analysis.cpp
)

add_executable(IO_Tests
  ${IO_Tests}
  io_testing_support.cpp
  )

target_link_libraries(IO_Tests ${TESTING_DEPENDECIES} EspinaPluginTesting )

add_test("\"IO: Load Default Classification\""               IO_Tests io_load_default_classification)
add_test("\"IO: Classification XML IO\""                     IO_Tests io_classification_xml_io)
add_test("\"IO: Analysis Seg File IO\""                      IO_Tests io_analysis_seg_file_io)
add_test("\"IO: Load Seg File Without Registered Filters\""  IO_Tests io_load_seg_file_analysis_without_registered_filters)
add_test("\"IO: Save Merged Analysis\""                      IO_Tests io_save_merged_analysis)
add_test("\"IO: SGS SAS\""                                   IO_Tests io_sgs_sas)
# add_test("\"IO: Skeleton\""                                  IO_Tests io_skeleton)
