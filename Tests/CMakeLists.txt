enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build ESPINA Unit Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use ESPINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()

set( CORE_DIR       ${CMAKE_SOURCE_DIR}/Core       )
set( GUI_DIR        ${CMAKE_SOURCE_DIR}/GUI        )
set( EXTENSIONS_DIR ${CMAKE_SOURCE_DIR}/Extensions )
set( FILTERS_DIR    ${CMAKE_SOURCE_DIR}/Filters    )
set( APP_DIR        ${CMAKE_SOURCE_DIR}/App        )
set( PLUGINS_DIR    ${CMAKE_SOURCE_DIR}/Plugins    )

include_directories(
  .
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/Plugins/AppositionSurface
  )

set(EXTERNAL_LIBS_DEPENDENCIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
)

# Core
qt4_add_resources(CORE_RCCS
  ${CORE_DIR}/rsc/core.qrc
  )
qt4_wrap_cpp(CORE_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/Extensions.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)
set (CORE_SOURCES
  ${CORE_RCCS}
  ${CORE_MOCS}
  ${CORE_DIR}/Analysis/Data/MeshData.cpp
#   ${CORE_DIR}/Analysis/Data/SkeletonData.cpp
  ${CORE_DIR}/Utils/vtkPolyDataUtils.cpp
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Data/Mesh/RawMesh.cpp
  ${CORE_DIR}/Analysis/Data/Mesh/MarchingCubesMesh.cpp
  ${CORE_DIR}/Analysis/Data/VolumetricData.cpp
  ${CORE_DIR}/Analysis/Data/Volumetric/ROI.cpp
#   ${CORE_DIR}/Analysis/Data/Skeleton/RawSkeleton.cpp
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
  ${CORE_DIR}/IO/DataFactory/RawDataFactory.cpp
  ${CORE_DIR}/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.cpp
  ${CORE_DIR}/IO/DataFactory/RasterizedVolumeFromFetchedMeshData.cpp
  ${CORE_DIR}/IO/GraphIO.cpp
  ${CORE_DIR}/IO/ReadOnlyChannelExtension.cpp
  ${CORE_DIR}/IO/ReadOnlySegmentationExtension.cpp
  ${CORE_DIR}/IO/SegFile.cpp
  ${CORE_DIR}/IO/SegFileInterface.cpp
  ${CORE_DIR}/IO/SegFile_V4.cpp
  ${CORE_DIR}/IO/SegFile_V5.cpp
  ${CORE_DIR}/IO/ZipUtils.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/AnalysisUtils.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/EspinaException.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${CORE_DIR}/Utils/vtkPolyDataUtils.cpp
)
add_library(EspinaCoreTesting SHARED ${CORE_SOURCES})
target_link_libraries(EspinaCoreTesting ${EXTERNAL_LIBS_DEPENDENCIES} )


# Filters
qt4_wrap_cpp(FILTERS_MOCS
  ${FILTERS_DIR}/MorphologicalEditionFilter.h
  ${FILTERS_DIR}/SeedGrowSegmentationFilter.h
)
set (FILTERS_SOURCES
  ${FILTERS_MOCS}
  ${FILTERS_DIR}/DilateFilter.cpp
  ${FILTERS_DIR}/ImageLogicFilter.cpp
  ${FILTERS_DIR}/MorphologicalEditionFilter.cpp
  ${FILTERS_DIR}/SeedGrowSegmentationFilter.cpp
  ${FILTERS_DIR}/SourceFilter.cpp
  ${FILTERS_DIR}/SplitFilter.cpp
  ${FILTERS_DIR}/Utils/Stencil.cpp
  ${FILTERS_DIR}/VolumetricStreamReader.cpp
)
add_library(EspinaFiltersTesting SHARED ${FILTERS_SOURCES})
target_link_libraries(EspinaFiltersTesting ${EXTERNAL_LIBS_DEPENDENCIES} )


# Extensions
set (EXTENSIONS_SOURCES
  ${EXTENSIONS_DIR}/Issues/Issues.cpp
  ${EXTENSIONS_DIR}/Issues/SegmentationIssues.cpp
  ${EXTENSIONS_DIR}/Notes/SegmentationNotes.cpp
)
add_library(EspinaExtensionsTesting SHARED ${EXTENSIONS_SOURCES})
target_link_libraries(EspinaExtensionsTesting ${EXTERNAL_LIBS_DEPENDENCIES} )


# GUI
qt4_wrap_cpp(GUI_MOCS
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/ModelAdapter.h
  ${GUI_DIR}/Model/Proxies/ChannelProxy.h
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.h
  ${GUI_DIR}/Model/ViewItemAdapter.h
  ${GUI_DIR}/Utils/Timer.h
  ${GUI_DIR}/View/ViewState.h
  ${GUI_DIR}/View/CoordinateSystem.h
  ${GUI_DIR}/View/EventHandler.h
  ${GUI_DIR}/View/Selection.h
)

set (GUI_SOURCES
  ${GUI_MOCS}
  ${GUI_DIR}/ColorEngines/IntensitySelectionHighlighter.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  ${GUI_DIR}/Model/ModelAdapter.cpp
  ${GUI_DIR}/Model/NeuroItemAdapter.cpp
  ${GUI_DIR}/Model/Proxies/ChannelProxy.cpp
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/Utils/ModelUtils.cpp
  ${GUI_DIR}/Model/Utils/SegmentationUtils.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/Frame.cpp
  ${GUI_DIR}/SupportedFormats.cpp
  ${GUI_DIR}/Utils/Timer.cpp
  ${GUI_DIR}/View/CoordinateSystem.cpp
  ${GUI_DIR}/View/EventHandler.cpp
  ${GUI_DIR}/View/Selection.cpp
  ${GUI_DIR}/View/ViewState.cpp
)
add_library(EspinaGUITesting SHARED ${GUI_SOURCES})
target_link_libraries(EspinaGUITesting ${EXTERNAL_LIBS_DEPENDENCIES} EspinaExtensionsTesting)

qt4_wrap_cpp(PLUGIN_MOCS
  ${PLUGINS_DIR}/AppositionSurface/Filter/AppositionSurfaceFilter.h
)

set (PLUGIN_SOURCES
  ${PLUGIN_MOCS}
  ${PLUGINS_DIR}/AppositionSurface/Filter/AppositionSurfaceFilter.cpp
)
add_library(EspinaPluginTesting SHARED ${PLUGIN_SOURCES})
target_link_libraries(EspinaPluginTesting ${EXTERNAL_LIBS_DEPENDENCIES} )

# Testing
qt4_wrap_cpp(TESTING_MOCS
  ModelProfiler.h
  ModelTest.h
)

set (TESTING_SOURCES
  ${TESTING_MOCS}
  testing_support_channel_input.cpp
  testing_support_dummy_filter.cpp
  MeshTestingUtils.cpp
  ModelProfiler.cpp
  ModelTest.cpp
  ModelTestUtils.cpp
#   SkeletonTestingUtils.cpp
  Testing_Support.cpp
)
add_library(EspinaTesting SHARED ${TESTING_SOURCES})

set(TESTING_DEPENDECIES
  EspinaCoreTesting
  EspinaExtensionsTesting
  EspinaFiltersTesting
  EspinaGUITesting
  EspinaTesting
  ${EXTERNAL_LIBS_DEPENDENCIES}
  )

if (UNIX)
  set (TESTING_DEPENDECIES
    ${TESTING_DEPENDECIES}
    QtTest
  )
endif (UNIX)


if (BUILD_UNIT_TESTS)

  add_subdirectory(Core)
  add_subdirectory(Filters)
  add_subdirectory(GUI)
  add_subdirectory(Pipeline)
  add_subdirectory(IO)

endif(BUILD_UNIT_TESTS)
