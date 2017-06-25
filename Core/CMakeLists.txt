find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

# Add Qt Resource files
qt4_add_resources(CORE_RCC
  rsc/core.qrc
  )

qt4_wrap_cpp(CORE_MOC
  Analysis/Data.h
  Analysis/Extensions.h
  Analysis/Output.h
  Analysis/ViewItem.h
  IO/ProgressReporter.h
  IO/SaveThread.h
  MultiTasking/Scheduler.h
  MultiTasking/Task.h
  MultiTasking/TaskGroupProgress.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (CORE_SOURCES
  ${CORE_RCC}
  ${CORE_MOC}
  ${CORE_UI}
  Analysis/Analysis.cpp
  Analysis/Category.cpp
  Analysis/Channel.cpp
  Analysis/Connections.cpp
  Analysis/Data.cpp
  Analysis/Data/MeshData.cpp
  Analysis/Data/Mesh/RawMesh.cpp
  Analysis/Data/Mesh/MarchingCubesMesh.cpp
  Analysis/Data/SkeletonData.cpp
  Analysis/Data/SkeletonDataUtils.cpp
  Analysis/Data/Skeleton/RawSkeleton.cpp
  Analysis/Data/VolumetricData.cpp
  Analysis/Data/Volumetric/ROI.cpp
  Analysis/Filter.cpp
  Analysis/Graph/DirectedGraph.cpp
  Analysis/Input.cpp
  Analysis/Output.cpp
  Analysis/Query.cpp
  Analysis/Sample.cpp
  Analysis/Segmentation.cpp
  Analysis/ViewItem.cpp
  Analysis/Filters/SourceFilter.cpp
  Analysis/Filters/VolumetricStreamReader.cpp
  Factory/CoreFactory.cpp
  IO/ClassificationXML.cpp
  IO/DataFactory/RawDataFactory.cpp
  IO/DataFactory/MarchingCubesFromFetchedVolumetricData.cpp
  IO/DataFactory/RasterizedVolumeFromFetchedMeshData.cpp
  IO/GraphIO.cpp
  IO/SaveThread.cpp
  IO/SegFile.cpp
  IO/SegFileInterface.cpp
  IO/SegFile_V4.cpp
  IO/SegFile_V5.cpp
  IO/ZipUtils.cpp
  MultiTasking/Scheduler.cpp
  MultiTasking/Task.cpp
  MultiTasking/TaskGroupProgress.cpp
  Utils/AnalysisUtils.cpp
  Utils/Bounds.cpp
  Utils/EspinaException.cpp
  Utils/Measure.cpp
  Utils/SparseBinaryVolume.cpp
  Utils/SupportedFormats.cpp
  Utils/TemporalStorage.cpp
  Utils/VolumeBounds.cpp
  Utils/vtkPolyDataUtils.cpp
  Utils/vtkVoxelContour2D.cpp  
  Utils/BlockTimer.cpp
  )

set(CORE_EXTERNAL_LIBS
  ${QT_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW)
  SET (WIN64_EXTRA_LIBS bfd iberty imagehlp z)
  SET (CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${WIN64_EXTRA_LIBS})
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (EXTRA_LIBS bfd iberty imagehlp z)
    SET (CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(CORE_SOURCES ${CORE_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW)

add_library (EspinaCore SHARED ${CORE_SOURCES})

target_link_libraries (EspinaCore ${CORE_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
  EspinaCore
  BASE_NAME         EspinaCore
  EXPORT_MACRO_NAME EspinaCore_EXPORT
  EXPORT_FILE_NAME  EspinaCore_Export.h
  )

install(TARGETS EspinaCore
  # IMPORTANT: Add the EspinaCore library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")
