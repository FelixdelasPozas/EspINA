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
  ${CMAKE_SOURCE_DIR}/common/extensions/Margins
  ${CMAKE_SOURCE_DIR}/common/extensions/Morphological
  ${CMAKE_SOURCE_DIR}/common/widgets
  ${CMAKE_SOURCE_DIR}/common/views
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
#   ${CMAKE_CURRENT_BINARY_DIR}/widgets
  ${ITK_VTK_GLUE}
)

####### EspinaPVCore #########

ADD_3DWIDGET(
  WIDGET_IFACES WIDGET_SOURCES
  CLASS_NAME pqNonRotatingBoxWidget
  WIDGET_TYPE NonRotatingBox
  )

ADD_3DWIDGET(
  WIDGET2_IFACES WIDGET2_SOURCES
  CLASS_NAME pqRectangularWidget
  WIDGET_TYPE RectangularBorder
  )

ADD_PARAVIEW_VIEW_MODULE(
  SLICE_VIEW_IFACES SLICE_VIEW_SOURCES
  VIEW_TYPE pqSliceView
  VIEW_XML_GROUP views
  )

ADD_PARAVIEW_VIEW_MODULE(
  VOLUME_VIEW_IFACES VOLUME_VIEW_SOURCES
  VIEW_TYPE pqVolumeView
  VIEW_XML_GROUP views
  )

set (PVCORE_IFACES
  ${WIDGET_IFACES}
  ${WIDGET2_IFACES}
  ${SLICE_VIEW_IFACES}
  ${VOLUME_VIEW_IFACES}
  )

qt4_wrap_cpp(PVMOC_SOURCES
  views/pqSliceView.h
  views/pqVolumeView.h
  widgets/pqRectangularWidget.h
  widgets/pqNonRotatingBoxWidget.h
  )

set (PVCORE_SERVER_SOURCES
  extensions/Margins/vtkChannelBorderDetector.cpp
  extensions/Morphological/vtkMorphologicalFeaturesFilter.cpp
  views/vtkPVSliceView.cpp
  views/vtkPVVolumeView.cpp
  views/vtkSMSliceViewProxy.cpp
  views/vtkChannelRepresentation.cpp
  views/vtkSegmentationRepresentation.cpp
  views/vtkSliceRepresentation.cpp
  views/vtkVolumetricRepresentation.cpp
  widgets/vtkRectangularRepresentation.cpp
  widgets/vtkRectangularWidget.cpp
  widgets/vtkNonRotatingBoxRepresentation.cpp
  widgets/vtkNonRotatingBoxWidget.cpp
  )

set (PVCORE_SOURCES
  ${PVMOC_SOURCES}
  ${WIDGET_SOURCES}
  ${WIDGET2_SOURCES}
  ${SLICE_VIEW_SOURCES}
  ${VOLUME_VIEW_SOURCES}
  views/pqSliceView.cpp
  views/pqVolumeView.cpp
  widgets/pqRectangularWidget.cpp
  widgets/pqNonRotatingBoxWidget.cpp
  widgets/RectangularSelection.cpp
  )

set (PVCORE_XML
  extensions/Margins/Margins.xml
  extensions/Morphological/MorphologicalExtension.xml
  model/Model.xml
  widgets/EspinaWidgets.xml
  views/EspinaViews.xml
  )

ADD_PARAVIEW_PLUGIN(
  EspinaPVCore
  "${ESPINA_VERSION_MAJOR}.${ESPINA_VERSION_MINOR}"
  SERVER_MANAGER_XML ${PVCORE_XML}
  SERVER_MANAGER_SOURCES ${PVCORE_SERVER_SOURCES}
  SOURCES ${PVCORE_SOURCES}
  GUI_INTERFACES ${PVCORE_IFACES}
  )

set (PVCORE_LIBRARIES
  ITKCommon ITKBasicFilters ITKAlgorithms
  )

target_link_libraries(EspinaPVCore ${PVCORE_LIBRARIES})

####### EspinaCore #########

# Add extra library containing custom code for the client.
qt4_wrap_cpp(CORE_MOC
  EspinaCore.h
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
  model/proxies/SampleProxy.h
  model/proxies/TaxonomyProxy.h
  selection/SelectionHandler.h
  selection/SelectionManager.h
  settings/ColorEngineSettings.h
  settings/GeneralSettings.h
  )

set (CORE_SOURCES
  ${CORE_MOC}
  EspinaCore.cpp
  File.cpp
  cache/Cache.cpp
  cache/CacheHash.cpp
  cache/CachedObjectBuilder.cpp
  extensions/ChannelExtension.cpp
  extensions/Margins/MarginsChannelExtension.cpp
  extensions/Margins/MarginsSegmentationExtension.cpp
  extensions/ModelItemExtension.cpp
  extensions/Morphological/MorphologicalExtension.cpp
  extensions/SampleExtension.cpp
  extensions/SegmentationExtension.cpp
  gui/ActionSelector.cpp
  gui/ActionSelectorWidget.cpp
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/EspinaView.cpp
  gui/SliceView.cpp
  gui/TaxonomyColorEngine.cpp
  gui/UserColorEngine.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
  model/Channel.cpp
  model/EspinaFactory.cpp
  model/EspinaModel.cpp
  model/Filter.cpp
  model/ModelItem.cpp
  model/ModelTest.cpp
  model/RelationshipGraph.cpp
  model/Sample.cpp
  model/Segmentation.cpp
  model/Taxonomy.cpp
  model/proxies/SampleProxy.cpp
  model/proxies/TaxonomyProxy.cpp
  processing/pqData.cpp
  processing/pqFilter.cpp
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
  ${Boost_LIBRARIES}
  QtTest
  EspinaPVCore
  pqApplicationComponents
)

add_library (EspinaCore SHARED ${CORE_SOURCES})
target_link_libraries (EspinaCore ${CORE_EXTERNAL_LIBS})

#install (
#  TARGETS EspinaCore
#  LIBRARY
#  DESTINATION lib
#  COMPONENT espina
#  )

