find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)


include_directories(
  ${CMAKE_SOURCE_DIR}/common 
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR} # Generated EspinaConfig.h
  ${CMAKE_BINARY_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES
  toolbar/seedgrow/rsc/SeedGrowSegmentation.qrc
  toolbar/voi/rsc/VolumeOfInterest.qrc
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (MOC_BUILT_SOURCES
  ColorEngineMenu.h
  EspinaWindow.h
  SettingsDialog.h
  docks/DataView/DataView.h
  docks/DataView/QueryView.h
  docks/DataView/InformationProxy.h
  docks/FilterInspector/FilterInspector.h
  docks/ChannelExplorer.h
  docks/ChannelInspector.h
  docks/SegmentationExplorer.h
  docks/SegmentationInspector.h
  docks/TaxonomyExplorer.h
  toolbar/MainToolBar.h
  toolbar/editor/EditorToolBar.h
  toolbar/seedgrow/SeedGrowSegmentation.h
  toolbar/seedgrow/SeedGrowSegmentationFilter.h
  toolbar/seedgrow/SeedGrowSelector.h
  toolbar/seedgrow/SettingsPanel.h
  toolbar/seedgrow/SetupWidget.h
  toolbar/seedgrow/gui/DefaultVOIAction.h
  toolbar/seedgrow/gui/ThresholdAction.h
  toolbar/editor/SettingsPanel.h
  toolbar/voi/VolumeOfInterest.h
  toolbar/voi/RectangularVOI.h
  views/DefaultEspinaView.h
  views/SliceViewSettingsPanel.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  docks/DataView/DataView.ui
  docks/DataView/QueryView.ui
  docks/ChannelExplorer.ui
  docks/ChannelInspector.ui
  docks/SegmentationExplorer.ui
  docks/SegmentationInspector.ui
  docks/TaxonomyExplorer.ui
  toolbar/seedgrow/SetupWidget.ui
  toolbar/seedgrow/SettingsPanel.ui
  toolbar/editor/EditorToolBarSettingsPanel.ui
  views/SliceViewSettingsPanel.ui
  views/VolumeViewSettingsPanel.ui
  AboutDialog.ui
  GeneralSettingsPanel.ui
  SettingsDialog.ui
  )

set (DOCKS_SOURCES
  docks/DataView/DataView.cpp
  docks/DataView/DataViewPanel.cpp
  docks/DataView/QueryView.cpp
  docks/DataView/InformationProxy.cpp
  docks/FilterInspector/FilterInspector.cpp
  docks/ChannelExplorer.cpp
  docks/ChannelInspector.cpp
  docks/SegmentationDelegate.cpp
  docks/SegmentationExplorer.cpp
  docks/SegmentationInspector.cpp
  docks/TaxonomyExplorer.cpp
  views/SliceViewSettingsPanel.cpp
  views/VolumeViewSettingsPanel.cpp
  )

set (TOOLBAR_SOURCES
  toolbar/MainToolBar.cpp
  toolbar/editor/EditorToolBar.cpp
  toolbar/editor/Settings.cpp
  toolbar/editor/SettingsPanel.cpp
  toolbar/seedgrow/SeedGrowSegmentation.cpp
  toolbar/seedgrow/SeedGrowSegmentationFilter.cpp
  toolbar/seedgrow/SeedGrowSelector.cpp
  toolbar/seedgrow/Settings.cpp
  toolbar/seedgrow/SettingsPanel.cpp
  toolbar/seedgrow/SetupWidget.cpp
  toolbar/seedgrow/gui/DefaultVOIAction.cpp
  toolbar/seedgrow/gui/ThresholdAction.cpp
  toolbar/voi/VolumeOfInterest.cpp
  toolbar/voi/RectangularVOI.cpp
  )

set (VIEW_SOURCES
  views/DefaultEspinaView.cpp
  )

set (LOCAL_SOURCES
  main.cpp
  AboutDialog.cpp
  ColorEngineMenu.cpp
  EspinaWindow.cpp
  SettingsDialog.cpp
  RecentDocuments.cpp
  )

set (Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${DOCKS_SOURCES}
  ${VIEW_SOURCES}
  ${TOOLBAR_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set (Espina_LIBRARIES
  EspinaCore
  QVTK
  #QtGui
#   ${Boost_LIBRARIES}
)


add_executable(espina ${Espina_SOURCE_FILES})

target_link_libraries (espina ${Espina_LIBRARIES})

#install (
#  TARGETS espina
#  RUNTIME
#  DESTINATION bin
##  COMPONENT espina
#  )
#
#
#add_subdirectory (plugins)
