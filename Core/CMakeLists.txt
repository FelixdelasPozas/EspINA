include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

qt4_wrap_cpp(CORE_MOC
  ColorEngines/IColorEngine.h
  ColorEngines/TaxonomyColorEngine.h
  Model/Output.h
  Extensions/ChannelExtension.h
  Extensions/SegmentationExtension.h
  Model/EspinaModel.h
  Model/ModelTest.h
  Model/ModelItem.h
  Model/PickableItem.h
  Model/Output.h
  Model/OutputRepresentation.h
  Model/MarchingCubesMesh.h
  Model/Segmentation.h
  Model/Taxonomy.h
  Model/Proxies/ChannelProxy.h
  Model/Proxies/LocationProxy.h
  Model/Proxies/RelationProxy.h
  Model/Proxies/TaxonomyProxy.h
  Model/Proxies/InformationProxy.h
  Model/Proxies/ConnectomicProxy.h
  MultiTasking/Task.h
  MultiTasking/Scheduler.h
  OutputRepresentations/ChannelVolumeProxy.h
  OutputRepresentations/VolumeProxy.h
  Interfaces/IDockWidget.h
  Interfaces/IToolBar.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (CORE_SOURCES 
  ${CORE_MOC}
  ${CORE_UI}
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/TaxonomyColorEngine.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/UserColorEngine.cpp
  EspinaRegion.cpp
  EspinaSettings.cpp
  EspinaTypes.cpp
  EspinaTypes.cpp
  Extensions/ChannelExtension.cpp
  Extensions/EdgeDistances/AdaptiveEdges.cpp
  Extensions/EdgeDistances/EdgeDetector.cpp
  Extensions/EdgeDistances/EdgeDistance.cpp
  Extensions/ModelItemExtension.cpp
  Extensions/Morphological/MorphologicalInformation.cpp
  Extensions/Notes/SegmentationNotes.cpp 
  Extensions/SampleExtension.cpp
  Extensions/SegmentationExtension.cpp
  Extensions/Tags/TagExtension.cpp
  Filters/BasicSegmentationFilter.cpp
  Filters/ChannelReader.cpp
  Filters/ClosingFilter.cpp
  Filters/DilateFilter.cpp
  Filters/ErodeFilter.cpp
  Filters/FillHolesFilter.cpp
  Filters/FreeFormSource.cpp
  Filters/ImageLogicFilter.cpp
  Filters/MorphologicalEditionFilter.cpp
  Filters/OpeningFilter.cpp
  Filters/SeedGrowSegmentationFilter.cpp
  Filters/SplitFilter.cpp
  Filters/TubularSegmentationFilter.cpp
  IO/SegFileReader.cpp
  Interfaces/IColorEngineProvider.cpp
  Interfaces/IDockWidget.cpp
  Interfaces/IDynamicMenu.cpp
  Interfaces/IFactoryExtension.cpp
  Interfaces/IFileReader.cpp
  Interfaces/IFilterCreator.cpp
  Interfaces/IToolBar.cpp
  Interfaces/IToolBarFactory.cpp
  Model/Channel.cpp
  Model/EspinaFactory.cpp
  Model/EspinaModel.cpp
  Model/Filter.cpp
  Model/HierarchyItem.cpp
  Model/MarchingCubesMesh.cpp 
  Model/ModelItem.cpp
  Model/ModelTest.cpp
  Model/Output.cpp
  Model/OutputRepresentation.cpp
  Model/PickableItem.cpp
  Model/Proxies/ChannelProxy.cpp
  Model/Proxies/ConnectomicProxy.cpp
  Model/Proxies/InformationProxy.cpp
  Model/Proxies/LocationProxy.cpp
  Model/Proxies/RelationProxy.cpp
  Model/Proxies/TaxonomyProxy.cpp
  Model/QtModelUtils.cpp
  Model/RelationshipGraph.cpp
  Model/Sample.cpp
  Model/Segmentation.cpp
  Model/Taxonomy.cpp
  MultiTasking/Scheduler.cpp
  MultiTasking/Task.cpp
  OutputRepresentations/ChannelVolumeProxy.cpp 
  OutputRepresentations/MeshProxy.cpp 
  OutputRepresentations/MeshType.cpp
  OutputRepresentations/RasterizedVolume.cpp
  OutputRepresentations/RawMesh.cpp 
  OutputRepresentations/RawVolume.cpp
  OutputRepresentations/VolumeProxy.cpp 
  OutputRepresentations/VolumeRepresentation.cpp 
  Utils/Measure.cpp
  Utils/SegmentationCollision.cpp
  VTK/vtkTube.cxx
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
	  SET(CORE_SOURCES ${CORE_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
	  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
      ENABLE_LANGUAGE(RC)
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
