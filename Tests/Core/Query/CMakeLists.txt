# Analysis tests
create_test_sourcelist(Query_Tests Query_Tests.cpp # this file is created by this command
  query_sample_channels.cpp
  query_sample_segmentations.cpp
  query_segmentation_sample.cpp
  query_segmentation_channel.cpp
)

add_executable(Query_Tests "" ${Query_Tests})
target_link_libraries(Query_Tests ${CORE_DEPENDECIES} )

add_test("\"Query: Sample Channels\""      Query_Tests query_sample_channels)
add_test("\"Query: Sample Segmentations\"" Query_Tests query_sample_segmentations)
add_test("\"Query: Segmentation Sample\""  Query_Tests query_segmentation_sample)
add_test("\"Query: Segmentation Channel\"" Query_Tests query_segmentation_channel)