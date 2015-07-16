# SparseVolume tests
create_test_sourcelist(SparseVolume_Tests SparseVolume_Tests.cpp # this file is created by this command
  default_constructor.cpp
  bounds_constructor.cpp
  draw_implicit_function.cpp
  draw_implicit_function_with_bigger_bounds.cpp
  sparse_volume_resize_expand_volume.cpp
  sparse_volume_resize_reduce_volume.cpp
  sparse_volume_save_edited_regions.cpp
  sparse_volume_load_edited_regions.cpp
)

add_executable(SparseVolume_Tests "" ${SparseVolume_Tests})  #"" is a hack to display target on kdevelop

target_link_libraries(SparseVolume_Tests ${CORE_DEPENDECIES})

add_test("\"Sparse Volume: Default Constructor\""                       SparseVolume_Tests default_constructor)
add_test("\"Sparse Volume: Bounds Constructor\""                        SparseVolume_Tests bounds_constructor)
add_test("\"Sparse Volume: Draw Implicit Function\""                    SparseVolume_Tests draw_implicit_function)
add_test("\"Sparse Volume: Draw Implicit Function With Bigger Bounds\"" SparseVolume_Tests draw_implicit_function_with_bigger_bounds)
#add_test("\"Sparse Volume: Compact\""                                   SparseVolume_Tests compact)
add_test("\"Sparse Volume: Resize Expand Volume\""                      SparseVolume_Tests sparse_volume_resize_expand_volume)
add_test("\"Sparse Volume: Resize Reduce Volume\""                      SparseVolume_Tests sparse_volume_resize_reduce_volume)
add_test("\"Sparse Volume: Save Edited Regions\""                       SparseVolume_Tests sparse_volume_save_edited_regions)
add_test("\"Sparse Volume: Load Edited Regions\""                       SparseVolume_Tests sparse_volume_load_edited_regions)