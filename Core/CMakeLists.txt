include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(CORE_MOC
  ColorEngines/IColorEngine.h
  ColorEngines/TaxonomyColorEngine.h
  EspinaVolume.h
  Extensions/ModelItemExtension.h
  Model/EspinaModel.h
  Model/ModelTest.h
  Model/ModelItem.h
  Model/PickableItem.h
  Model/Segmentation.h
  Model/Taxonomy.h
  Model/Proxies/ChannelProxy.h
  Model/Proxies/CompositionProxy.h
  Model/Proxies/SampleProxy.h
  Model/Proxies/TaxonomyProxy.h
  Model/Proxies/InformationProxy.h
  Model/Proxies/ConnectomicProxy.h
  Interfaces/IDockWidget.h
  Interfaces/IToolBar.h
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
  EspinaVolume.cpp
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/TaxonomyColorEngine.cpp
  ColorEngines/UserColorEngine.cpp
  IO/EspinaIO.cpp
  Extensions/ChannelExtension.cpp
  Extensions/EdgeDistances/EdgeDetector.cpp
  Extensions/EdgeDistances/AdaptiveEdges.cpp
  Extensions/EdgeDistances/EdgeDistance.cpp
  Extensions/ModelItemExtension.cpp
  Extensions/Morphological/MorphologicalInformation.cpp
  Extensions/SampleExtension.cpp
  Extensions/SegmentationExtension.cpp
  Model/Channel.cpp
  Model/EspinaFactory.cpp
  Model/EspinaModel.cpp
  Model/Filter.cpp
  Model/QtModelUtils.cpp
  Model/ModelItem.cpp
  Model/ModelTest.cpp
  Model/PickableItem.cpp
  Model/RelationshipGraph.cpp
  Model/Sample.cpp
  Model/Segmentation.cpp
  Model/Taxonomy.cpp
  EspinaTypes.cpp
  Model/Proxies/ChannelProxy.cpp
  Model/Proxies/CompositionProxy.cpp
  Model/Proxies/SampleProxy.cpp
  Model/Proxies/TaxonomyProxy.cpp
  Model/Proxies/InformationProxy.cpp
  Model/Proxies/ConnectomicProxy.cpp
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
