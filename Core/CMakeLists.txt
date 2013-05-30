include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

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
  OutputRepresentations/ChannelVolumeProxy.h
  OutputRepresentations/VolumeProxy.h
  Interfaces/IDockWidget.h
  Interfaces/IToolBar.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

# qt4_wrap_ui (CORE_UI
#   gui/HueSelector.ui
#   editor/ContourSourceInspector.ui
#   )

set (CORE_SOURCES 
  ${CORE_MOC}
  ${CORE_UI}
  EspinaRegion.cpp
  EspinaSettings.cpp
  Model/Output.cpp
  Model/OutputRepresentation.cpp
  EspinaTypes.cpp
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/TaxonomyColorEngine.cpp
  ColorEngines/UserColorEngine.cpp
  IO/SegFileReader.cpp
  Extensions/ChannelExtension.cpp
  Extensions/EdgeDistances/EdgeDetector.cpp
  Extensions/EdgeDistances/AdaptiveEdges.cpp
  Extensions/EdgeDistances/EdgeDistance.cpp
  Extensions/ModelItemExtension.cpp
  Extensions/Morphological/MorphologicalInformation.cpp
  Extensions/SampleExtension.cpp
  Extensions/SegmentationExtension.cpp
  Extensions/Notes/SegmentationNotes.cpp 
  Extensions/Tags/TagExtension.cpp
  Model/Channel.cpp
  Model/EspinaFactory.cpp
  Model/EspinaModel.cpp
  Model/Filter.cpp
  Model/QtModelUtils.cpp
  Model/MarchingCubesMesh.cpp 
  Model/ModelItem.cpp
  Model/ModelTest.cpp
  Model/PickableItem.cpp
  Model/RelationshipGraph.cpp
  Model/Sample.cpp
  Model/Segmentation.cpp
  Model/Taxonomy.cpp
  EspinaTypes.cpp
  Model/Proxies/ChannelProxy.cpp
  Model/Proxies/LocationProxy.cpp
  Model/Proxies/TaxonomyProxy.cpp
  Model/Proxies/RelationProxy.cpp
  Model/Proxies/InformationProxy.cpp
  Model/Proxies/ConnectomicProxy.cpp
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
  OutputRepresentations/ChannelVolumeProxy.cpp 
  OutputRepresentations/MeshType.cpp
  OutputRepresentations/MeshProxy.cpp 
  OutputRepresentations/RawMesh.cpp 
  OutputRepresentations/RawVolume.cpp
  OutputRepresentations/RasterizedVolume.cpp
  OutputRepresentations/VolumeProxy.cpp 
  OutputRepresentations/VolumeRepresentation.cpp 
  VTK/vtkTube.cxx
  Utils/SegmentationCollision.cpp
  Model/HierarchyItem.cpp
  Utils/Measure.cpp
  )

set(CORE_EXTERNAL_LIBS
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

add_library (EspinaCore SHARED ${CORE_SOURCES})
target_link_libraries (EspinaCore ${CORE_EXTERNAL_LIBS})
