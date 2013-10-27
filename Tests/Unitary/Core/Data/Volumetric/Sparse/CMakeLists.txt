# SparseVolume tests
create_test_sourcelist(SparseVolume_Tests SparseVolume_Tests.cpp # this file is created by this command
  sparse_volume_default_constructor.cpp
  sparse_volume_bounds_constructor.cpp
  sparse_volume_draw_implicit_function.cpp
  sparse_volume_draw_implicit_function_non_inclusive_bounds.cpp
  sparse_volume_resize_expand_volume.cpp
  sparse_volume_resize_reduce_volume.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

add_executable(SparseVolume_Tests
  ${SparseVolume_Tests}
  ../Testing_Support.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/BinaryMask.cpp
  )

target_link_libraries(SparseVolume_Tests ${LIB_DEPENDECIES})

add_test("\"Sparse Volume: Default Constructor\""                         SparseVolume_Tests sparse_volume_default_constructor)
add_test("\"Sparse Volume: Bounds Constructor\""                          SparseVolume_Tests sparse_volume_bounds_constructor)
add_test("\"Sparse Volume: Draw Implicit Function\""                      SparseVolume_Tests sparse_volume_draw_implicit_function)
add_test("\"Sparse Volume: Draw Implicit Function Non Inclusive Bounds\"" SparseVolume_Tests sparse_volume_draw_implicit_function_non_inclusive_bounds)
add_test("\"Sparse Volume: Resize Expand Volume\""                        SparseVolume_Tests sparse_volume_resize_expand_volume)
add_test("\"Sparse Volume: Resize Reduce Volume\""                        SparseVolume_Tests sparse_volume_resize_reduce_volume)