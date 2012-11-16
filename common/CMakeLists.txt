find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})
#NOTE: We can't use ITK and BOOST graph at the same time because of
#      ITK_REQUIRED_CXX_FLAG: -ftemplate-depth-50
#      so we override CXX_FLAGS to unset previous flag
#set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -msse2")
set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated")

find_package(Boost REQUIRED COMPONENTS  graph) #system filesystem
link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_SOURCE_DIR}/common/editor
  ${CMAKE_SOURCE_DIR}/common/gui
  ${CMAKE_SOURCE_DIR}/common/extensions/Margins
  ${CMAKE_SOURCE_DIR}/common/extensions/Morphological
  ${CMAKE_SOURCE_DIR}/common/widgets
  ${CMAKE_SOURCE_DIR}/common/views
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  ${CMAKE_BINARY_DIR} # Generated EspinaConfig.h
)

####### EspinaCore #########
# Add extra library containing custom code for the client.
qt4_wrap_cpp(CORE_MOC
  colorEngines/ColorEngine.h
  editor/CODESettings.h
  gui/ActionSelector.h
  gui/ActionSelectorWidget.h
  gui/EspinaRenderView.h
  gui/Renderer.h
  gui/SliceView.h
  gui/VolumeView.h
  gui/QComboTreeView.h
  gui/HueSelector.h
  model/EspinaModel.h
  model/ModelTest.h
  model/ModelItem.h
  model/Segmentation.h
  model/proxies/ChannelProxy.h
  model/proxies/SampleProxy.h
  model/proxies/TaxonomyProxy.h
  tools/IPicker.h
  tools/BrushPicker.h
  gui/ViewManager.h
  editor/ContourWidget.h
  widgets/RectangularRegion.h
  widgets/RectangularRegionSliceSelector.h
  editor/ContourSourceInspector.h
  )

qt4_wrap_ui (CORE_UI
  gui/HueSelector.ui
  editor/ContourSourceInspector.ui
  )

set (CORE_SOURCES
  ${CORE_MOC}
  ${CORE_UI}
  BoundingBox.cpp
  EspinaRegions.cpp
  colorEngines/MultiColorEngine.cpp
  colorEngines/NumberColorEngine.cpp
  colorEngines/TransparencySelectionHighlighter.cpp
  colorEngines/TaxonomyColorEngine.cpp
  colorEngines/UserColorEngine.cpp
  editor/EspinaVolume.cpp
  editor/CODESettings.cpp
  editor/ContourSelector.cpp
  editor/FillHolesFilter.cpp
  editor/FillHolesCommand.cpp
  editor/FreeFormSource.cpp
  editor/ImageLogicCommand.cpp
  editor/MorphologicalEditionFilter.cpp
  editor/ClosingFilter.cpp
  editor/OpeningFilter.cpp
  editor/DilateFilter.cpp
  editor/ErodeFilter.cpp
  editor/ImageLogicFilter.cpp
  editor/vtkTube.cxx
  editor/split/SplitFilter.cpp
  IO/EspinaIO.cpp
  extensions/Margins/MarginDetector.cpp
  extensions/Margins/MarginsChannelExtension.cpp
  extensions/Margins/MarginsSegmentationExtension.cpp
  extensions/ModelItemExtension.cpp
  extensions/Morphological/MorphologicalExtension.cpp
  extensions/SampleExtension.cpp
  gui/ActionSelector.cpp
  gui/ActionSelectorWidget.cpp
  gui/EspinaRenderView.cpp
  gui/QComboTreeView.cpp
  gui/SliceView.cpp
  gui/SliceViewState.cpp
  gui/vtkInteractorStyleEspinaSlice.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
  gui/HueSelector.cpp
  model/Channel.cpp
  model/ChannelReader.cpp
  model/EspinaFactory.cpp
  model/EspinaModel.cpp
  model/Filter.cpp
  model/ModelItem.cpp
  model/ModelTest.cpp
  model/RelationshipGraph.cpp
  model/Sample.cpp
  model/Segmentation.cpp
  model/Taxonomy.cpp
  EspinaTypes.cpp
  model/proxies/ChannelProxy.cpp
  model/proxies/SampleProxy.cpp
  model/proxies/TaxonomyProxy.cpp
  renderers/CrosshairRenderer.cpp
  renderers/VolumetricRenderer.cpp
  renderers/MeshRenderer.cpp
  tools/PixelSelector.cpp
  tools/BrushPicker.cpp
  tools/IPicker.cpp
  settings/GeneralSettings.cpp
  undo/AddChannel.cpp
  undo/AddFilter.cpp
  undo/AddRelation.cpp
  undo/AddSample.cpp
  undo/AddSegmentation.cpp
  undo/RemoveSegmentation.cpp
  widgets/RectangularRegionSliceSelector.cpp
  widgets/vtkRectangularSliceRepresentation.cpp
  widgets/vtkRectangularSliceWidget.cpp
  widgets/EspinaInteractorAdapter.cpp
  widgets/EspinaWidget.cpp
  widgets/RectangularRegion.cpp
  editor/ContourWidget.cpp
  editor/vtkPlaneContourWidget.cpp
  editor/vtkPlaneContourRepresentation.cpp
  editor/vtkPlaneContourRepresentationGlyph.cpp
  editor/SliceContourWidget.cpp
  editor/vtkClipClosedSurface.cxx
  editor/vtkContourToPolygonFilter.cxx
  editor/ContourSource.cpp
  editor/ContourSourceInspector.cpp
  )

set(CORE_EXTERNAL_LIBS
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  QVTK
  QtTest
)

add_library (EspinaCore SHARED ${CORE_SOURCES})
target_link_libraries (EspinaCore ${CORE_EXTERNAL_LIBS})
