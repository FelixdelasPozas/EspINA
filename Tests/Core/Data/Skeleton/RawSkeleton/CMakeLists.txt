create_test_sourcelist(RawSkeleton_Tests RawSkeleton_Tests.cpp # this file is created by this command
  raw_skeleton_default_constructor.cpp
  raw_skeleton_constructor.cpp
  raw_skeleton_save_edited_regions.cpp
  raw_skeleton_load_edited_regions.cpp
  raw_skeleton_set_spacing.cpp
  raw_skeleton_set_skeleton.cpp
  raw_skeleton_utils_test.cpp
)

add_executable(RawSkeleton_Tests "" ${RawSkeleton_Tests})

target_link_libraries(RawSkeleton_Tests ${CORE_DEPENDECIES})

add_test("\"Raw Skeleton: Default Constructor\"" RawSkeleton_Tests raw_skeleton_default_constructor)
add_test("\"Raw Skeleton: Constructor\""         RawSkeleton_Tests raw_skeleton_constructor)
add_test("\"Raw Skeleton: Save Edited Regions\"" RawSkeleton_Tests raw_skeleton_save_edited_regions)
add_test("\"Raw Skeleton: Load Edited Regions\"" RawSkeleton_Tests raw_skeleton_load_edited_regions)
add_test("\"Raw Skeleton: Set Spacing\""         RawSkeleton_Tests raw_skeleton_set_spacing)
add_test("\"Raw Skeleton: Set Skeleton\""        RawSkeleton_Tests raw_skeleton_set_skeleton)
add_test("\"Raw Skeleton: Utils test\""          RawSkeleton_Tests raw_skeleton_utils_test)