include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

qt4_wrap_cpp(CORE_MOC
  Analysis/Data.h
  Analysis/Extension.h
  Analysis/Output.h
  Analysis/ViewItem.h
  MultiTasking/Scheduler.h
  MultiTasking/Task.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (CORE_SOURCES
  ${CORE_MOC}
  ${CORE_UI}
  Analysis/Analysis.cpp
  Analysis/Category.cpp
  Analysis/Channel.cpp
  Analysis/Data.cpp
  Analysis/Data/MeshData.cpp
  Analysis/Data/Mesh/RawMesh.cpp
  Analysis/Data/VolumetricData.cpp
  Analysis/Filter.cpp
  Analysis/Graph/DirectedGraph.cpp
  Analysis/Input.cpp
  Analysis/Output.cpp
  Analysis/Query.cpp
  Analysis/Sample.cpp
  Analysis/Segmentation.cpp
  Analysis/ViewItem.cpp
  Factory/CoreFactory.cpp
  IO/ClassificationXML.cpp
  IO/FetchBehaviour/FetchRawData.cpp
  IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.cpp
  IO/GraphIO.cpp
  IO/ReadOnlyChannelExtension.cpp
  IO/ReadOnlySegmentationExtension.cpp
  IO/SegFile.cpp
  IO/SegFileInterface.cpp
  IO/SegFile_V4.cpp
  IO/SegFile_V5.cpp
  MultiTasking/Scheduler.cpp
  MultiTasking/Task.cpp
  Utils/AnalysisUtils.cpp
  Utils/Bounds.cpp
  Utils/Measure.cpp
  Utils/NmVector3.cpp
  Utils/SparseBinaryVolume.cpp
  Utils/TemporalStorage.cpp
  Utils/VolumeBounds.cpp
  Utils/vtkPolyDataUtils.cpp
  )

set(CORE_EXTERNAL_LIBS
  ${QT_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW OR DEFINED MSVC)

  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(CORE_SOURCES ${CORE_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)

endif(DEFINED MINGW OR DEFINED MSVC)


add_library (EspinaCore SHARED ${CORE_SOURCES})

GENERATE_EXPORT_HEADER(
  EspinaCore
  BASE_NAME EspinaCore
  EXPORT_MACRO_NAME EspinaCore_EXPORT
  EXPORT_FILE_NAME EspinaCore_Export.h
  )
target_link_libraries (EspinaCore ${CORE_EXTERNAL_LIBS})