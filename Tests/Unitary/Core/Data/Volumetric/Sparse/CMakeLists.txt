# SparseVolume tests
create_test_sourcelist(SparseVolume_Tests SparseVolume_Tests.cpp # this file is created by this command
  default_constructor.cpp
  bounds_constructor.cpp
  draw_implicit_function.cpp
  draw_implicit_function_with_bigger_bounds.cpp
  #compact.cpp
#   sparse_volume_resize_expand_volume.cpp
#   sparse_volume_resize_reduce_volume.cpp
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(SparseVolume_Tests
  ${SparseVolume_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ../../../../Testing_Support.cpp
  )

  set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)


target_link_libraries(SparseVolume_Tests ${LIB_DEPENDECIES})

add_test("\"Sparse Volume: Default Constructor\""                       SparseVolume_Tests default_constructor)
add_test("\"Sparse Volume: Bounds Constructor\""                        SparseVolume_Tests bounds_constructor)
add_test("\"Sparse Volume: Draw Implicit Function\""                    SparseVolume_Tests draw_implicit_function)
add_test("\"Sparse Volume: Draw Implicit Function With Bigger Bounds\"" SparseVolume_Tests draw_implicit_function_with_bigger_bounds)
#add_test("\"Sparse Volume: Compact\""                                   SparseVolume_Tests compact)
# add_test("\"Sparse Volume: Resize Expand Volume\""                        SparseVolume_Tests sparse_volume_resize_expand_volume)
# add_test("\"Sparse Volume: Resize Reduce Volume\""                        SparseVolume_Tests sparse_volume_resize_reduce_volume)