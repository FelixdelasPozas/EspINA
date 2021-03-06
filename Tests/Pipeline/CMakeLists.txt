# Analysis tests
create_test_sourcelist(Pipeline_Tests Pipeline_Tests.cpp # this file is created by this command
  pipeline_access_internal_filter_edited_data.cpp
  pipeline_access_internal_filter_edited_data_with_fetch_behaviour.cpp
  pipeline_access_internal_filter_non_edited_data.cpp
  pipeline_image_logic_filter_addition.cpp
  pipeline_keep_edited_regions_on_save_filters_without_update.cpp
  pipeline_single_filter.cpp
  pipeline_single_filter_raw_fetch_behaviour.cpp
  pipeline_single_filter_raw_fetch_behaviour_partial_data_invalid_update.cpp
  pipeline_single_filter_raw_fetch_behaviour_partial_data_valid_update.cpp
  pipeline_single_read_only_filter_raw_fetch_behaviour.cpp
  pipeline_update_filter_with_edited_region_filter_as_input.cpp
  pipeline_update_filter_with_no_edited_region_filter_as_input.cpp
)

add_executable(Pipeline_Tests "" ${Pipeline_Tests} )

target_link_libraries(Pipeline_Tests ${TESTING_DEPENDECIES})

add_test("\"Pipeline: Single Filter\""                                                 Pipeline_Tests pipeline_single_filter)
add_test("\"Pipeline: Single Filter Raw Fetch Behaviour\""                             Pipeline_Tests pipeline_single_filter_raw_fetch_behaviour)
add_test("\"Pipeline: Single Read Only Filter Raw Fetch Behaviour\""                   Pipeline_Tests pipeline_single_read_only_filter_raw_fetch_behaviour)
add_test("\"Pipeline: Single Filter Raw Fetch Behaviour Partial Data Valid Update\""   Pipeline_Tests pipeline_single_filter_raw_fetch_behaviour_partial_data_valid_update)
add_test("\"Pipeline: Single Filter Raw Fetch Behaviour Partial Data Invalid Update\"" Pipeline_Tests pipeline_single_filter_raw_fetch_behaviour_partial_data_valid_update)
add_test("\"Pipeline: Access Internal Filter Non Edited Data\""                        Pipeline_Tests pipeline_access_internal_filter_non_edited_data)
add_test("\"Pipeline: Access Internal Filter Edited Data\""                            Pipeline_Tests pipeline_access_internal_filter_edited_data)
add_test("\"Pipeline: Access Internal Filter Edited Data With Fetch Behaviour\""       Pipeline_Tests pipeline_access_internal_filter_edited_data_with_fetch_behaviour)
add_test("\"Pipeline: Image Logic Filter Addition\""                                   Pipeline_Tests pipeline_image_logic_filter_addition)
add_test("\"Pipeline: Update Filter With Edited Region Filter As Input\""              Pipeline_Tests pipeline_update_filter_with_edited_region_filter_as_input)
add_test("\"Pipeline: Update Filter With No Edited Region Filter As Input\""           Pipeline_Tests pipeline_update_filter_with_no_edited_region_filter_as_input)
add_test("\"Pipeline: Keep Edited Regions On Save Filters Without Update\""            Pipeline_Tests pipeline_keep_edited_regions_on_save_filters_without_update)
