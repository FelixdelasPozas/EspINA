enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build ESPINA Unit Tests")

set (BUILD_CI_TESTS   OFF  CACHE BOOL "Build ESPINA Continous Integration Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use ESPINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  .
  )

set( CORE_DIR       ${CMAKE_SOURCE_DIR}/Core       )
set( GUI_DIR        ${CMAKE_SOURCE_DIR}/GUI        )
set( EXTENSIONS_DIR ${CMAKE_SOURCE_DIR}/Extensions )
set( FILTERS_DIR    ${CMAKE_SOURCE_DIR}/Filters    )
set( APP_DIR        ${CMAKE_SOURCE_DIR}/App        )

qt4_add_resources(CORE_RCCS
  ${CORE_DIR}/rsc/core.qrc
  )

qt4_wrap_cpp(CORE_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)
set (CORE_SOURCES
  ${CORE_RCCS}
  ${CORE_MOCS}
  ${CORE_DIR}/Analysis/Data/MeshData.cpp
  ${CORE_DIR}/Utils/vtkPolyDataUtils.cpp
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Data/Mesh/RawMesh.cpp
  ${CORE_DIR}/Analysis/Data/VolumetricData.cpp
  ${CORE_DIR}/Analysis/Data/Volumetric/ROI.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Query.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/IO/ClassificationXML.cpp
  ${CORE_DIR}/IO/FetchBehaviour/FetchRawData.cpp
  ${CORE_DIR}/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.cpp
  ${CORE_DIR}/IO/GraphIO.cpp
  ${CORE_DIR}/IO/ReadOnlyChannelExtension.cpp
  ${CORE_DIR}/IO/ReadOnlySegmentationExtension.cpp
  ${CORE_DIR}/IO/SegFile.cpp
  ${CORE_DIR}/IO/SegFileInterface.cpp
  ${CORE_DIR}/IO/SegFile_V4.cpp
  ${CORE_DIR}/IO/SegFile_V5.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/AnalysisUtils.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${CORE_DIR}/Utils/vtkPolyDataUtils.cpp
)

set (TESTING_SOURCES
  testing_support_channel_input.cpp
  testing_support_dummy_filter.cpp
  Testing_Support.cpp
)

add_library(EspinaCoreTesting SHARED ${CORE_SOURCES})
add_library(EspinaTesting     SHARED ${TESTING_SOURCES})

set(TESTING_DEPENDECIES
  EspinaCoreTesting
  EspinaTesting
  )

if (BUILD_UNIT_TESTS)

  add_subdirectory(Unitary)

endif(BUILD_UNIT_TESTS)

if (BUILD_CI_TESTS)

  add_subdirectory(Integration)

endif(BUILD_CI_TESTS)
