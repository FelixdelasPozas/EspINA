# Analysis tests
create_test_sourcelist(Pipeline_Tests Pipeline_Tests.cpp # this file is created by this command
  pipeline_update_input.cpp
  pipeline_update_request_non_edited_input.cpp
  pipeline_update_request_edited_input.cpp
  pipeline_update_request_edited_input_with_fetch_behaviour.cpp
)

add_executable(Pipeline_Tests
  ${Pipeline_Tests}
  ${FILTERS_DIR}/MorphologicalEditionFilter.cpp
  ${FILTERS_DIR}/DilateFilter.cpp
  ${FILTERS_DIR}/SeedGrowSegmentationFilter.cpp
  )

target_link_libraries(Pipeline_Tests ${TESTING_DEPENDECIES})

add_test("\"Pipeline: Update Input\""                                     Pipeline_Tests pipeline_update_input)
add_test("\"Pipeline: Update Request Non Edited Input\""                  Pipeline_Tests pipeline_update_request_non_edited_input)
add_test("\"Pipeline: Update Request Edited Input\""                      Pipeline_Tests pipeline_update_request_edited_input)
add_test("\"Pipeline: Update Request Edited Input With Fetch Behaviour\"" Pipeline_Tests pipeline_update_request_edited_input_with_fetch_behaviour)