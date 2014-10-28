# RawVolume tests
create_test_sourcelist(VolumetricUtils_Tests VolumetricUtils_Tests.cpp # this file is created by this command
  volumetric_utils_bounds_to_region.cpp
  volumetric_utils_slice_bounds_to_region.cpp
  volumetric_utils_region_to_bounds.cpp
  volumetric_utils_equivalent_bounds.cpp
  volumetric_utils_symmetric_equivalences.cpp
  volumetric_utils_create_itk_image.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  EspinaTesting
)
qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(VolumetricUtils_Tests
  ${VolumetricUtils_Tests}
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
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  )

target_link_libraries(VolumetricUtils_Tests ${LIB_DEPENDECIES})

add_test("\"Volumetric Utils: Bounds To Region\""       VolumetricUtils_Tests volumetric_utils_bounds_to_region)
add_test("\"Volumetric Utils: Slice Bounds To Region\"" VolumetricUtils_Tests volumetric_utils_slice_bounds_to_region)
add_test("\"Volumetric Utils: Region To Bounds\""       VolumetricUtils_Tests volumetric_utils_region_to_bounds)
add_test("\"Volumetric Utils: Equivalent Bounds\""      VolumetricUtils_Tests volumetric_utils_equivalent_bounds)
add_test("\"Volumetric Utils: Symmetric Equivalences\"" VolumetricUtils_Tests volumetric_utils_symmetric_equivalences)
add_test("\"Volumetric Utils: Create Itk Image\""       VolumetricUtils_Tests volumetric_utils_create_itk_image)