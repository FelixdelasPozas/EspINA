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
)

#   extensions/Margins/vtkChannelBorderDetector.cpp
#   extensions/Morphological/vtkMorphologicalFeaturesFilter.cpp


####### EspinaCore #########
# Add extra library containing custom code for the client.
qt4_wrap_cpp(CORE_MOC
  EspinaCore.h
  editor/PencilSelector.h
  editor/CODESettings.h
  gui/ActionSelector.h
  gui/ActionSelectorWidget.h
  gui/EspinaView.h
  gui/SliceView.h
  gui/ViewManager.h
  gui/VolumeView.h
  model/EspinaModel.h
  model/ModelTest.h
  model/ModelItem.h
  model/Segmentation.h
  model/proxies/ChannelProxy.h
  model/proxies/SampleProxy.h
  model/proxies/TaxonomyProxy.h
  pluginInterfaces/ToolBarInterface.h
  pluginInterfaces/Renderer.h
  selection/SelectionHandler.h
  selection/SelectionManager.h
  settings/ColorEngineSettings.h
  )

set (CORE_SOURCES
  ${CORE_MOC}
  EspinaCore.cpp
  cache/CacheHash.cpp
  editor/CODESettings.cpp
  editor/PencilSelector.cpp
  editor/FreeFormSource.cpp
  editor/ImageLogicCommand.cpp
  editor/MorphologicalEditionFilter.cpp
  editor/ClosingFilter.cpp
  editor/OpeningFilter.cpp
  editor/DilateFilter.cpp
  editor/ErodeFilter.cpp
  editor/ImageLogicFilter.cpp
  editor/vtkImageLogicFilter.cpp
  IO/FilePack.cpp
#   extensions/Margins/MarginsChannelExtension.cpp
#   extensions/Margins/MarginsSegmentationExtension.cpp
  extensions/ModelItemExtension.cpp
  extensions/Morphological/MorphologicalExtension.cpp
  extensions/SampleExtension.cpp
  gui/ActionSelector.cpp
  gui/ActionSelectorWidget.cpp
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/EspinaView.cpp
  gui/SliceView.cpp
  gui/SliceViewState.cpp
  gui/vtkInteractorStyleEspinaSlice.cpp
  gui/NumberColorEngine.cpp
  gui/TaxonomyColorEngine.cpp
  gui/UserColorEngine.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
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
  selection/PixelSelector.cpp
  selection/SelectionHandler.cpp
  selection/SelectionManager.cpp
  settings/ColorEngineSettings.cpp
  settings/GeneralSettings.cpp
  undo/AddChannel.cpp
  undo/AddFilter.cpp
  undo/AddRelation.cpp
  undo/AddSample.cpp
  undo/AddSegmentation.cpp
  undo/RemoveSegmentation.cpp
  #views/vtkSliceView.cpp
  #views/vtkVolumeView.cpp
  widgets/vtkRectangularRepresentation.cpp
  widgets/vtkRectangularWidget.cpp
  widgets/EspinaWidget.cpp
  widgets/RectangularSelection.cpp
  )

# Make debug output clearer
#foreach(arg ${CORE_SOURCES})
#   set(MSG_SOURCES "${MSG_SOURCES}\n *${arg}")
# endforeach(arg ${CORE_SOURCES})
#message("SOURCES:${MSG_SOURCES}")
#
#foreach(arg ${PVCORE_SOURCES})
#   set(MSG_PVSOURCES "${MSG_PVSOURCES}\n *${arg}")
# endforeach(arg ${PVCORE_SOURCES})
#message("PV SOURCES:${MSG_PVSOURCES}")
#
#foreach(arg ${PVCORE_XML})
#   set(MSG_PVXML "${MSG_PVXML}\n *${arg}")
# endforeach(arg ${PVCORE_XML})
#message("PV XML:${MSG_PVXML}")

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
