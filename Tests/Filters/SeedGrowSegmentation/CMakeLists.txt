# Seed Grow Segmentation Filter Tests
create_test_sourcelist(TEST_SOURCES SGSF_Tests.cpp # this file is created by this command
  seed_grow_segmentation_default_parameters.cpp
  seed_grow_segmentation_change_parameters.cpp
  seed_grow_segmentation_save_restore_state.cpp
  seed_grow_segmentation_basic_pipeline.cpp
  seed_grow_segmentation_change_spacing.cpp
  seed_grow_segmentation_change_spacing_restore_pipeline.cpp
)

add_executable(SGSF_Tests "" ${TEST_SOURCES} )


target_link_libraries(SGSF_Tests ${FILTERS_DEPENDECIES} )

add_test("\"Seed Grow Segmentation Filter: Default Parameters\"" SGSF_Tests seed_grow_segmentation_default_parameters)
add_test("\"Seed Grow Segmentation Filter: Change Parameters\""  SGSF_Tests seed_grow_segmentation_change_parameters)
add_test("\"Seed Grow Segmentation Filter: Save/Restore State\"" SGSF_Tests seed_grow_segmentation_save_restore_state)
add_test("\"Seed Grow Segmentation Filter: Basic Pipeline\""     SGSF_Tests seed_grow_segmentation_basic_pipeline)
add_test("\"Seed Grow Segmentation Filter: Change Spacing\""     SGSF_Tests seed_grow_segmentation_change_spacing)
add_test("\"Seed Grow Segmentation Filter: Change Spacing Restore Pipeline\""     SGSF_Tests seed_grow_segmentation_change_spacing_restore_pipeline)
