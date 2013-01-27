include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR} # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(ESPINA_RCC
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (ESPINA_MOC
  EspinaMainWindow.h
  Dialogs/SegmentationInspector/SegmentationInspector.h
  Dialogs/TabularReport/QueryView.h
  Dialogs/SettingsDialog.h
  Docks/ChannelExplorer/ChannelExplorer.h
  Docks/FilterInspector/FilterInspector.h
  Docks/SegmentationExplorer/SegmentationExplorer.h
  Docks/SegmentationExplorer/SegmentationExplorerLayout.h
  Docks/SegmentationExplorer/LayoutTaxonomy.h
  Docks/TabularReport/DataView.h
  FilterInspectors/CODE/CODEFilterInspector.h
  FilterInspectors/ContourSource/ContourInspectorWidget.h
  FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h
  Menus/ColorEngineMenu.h
  Settings/Editor/SettingsPanel.h
  Settings/SeedGrowSegmentation/SettingsPanel.h
  Settings/SliceView/SliceViewSettingsPanel.h
  Settings/VOI/SettingsPanel.h
  Toolbars/Composition/CompositionToolBar.h
  Toolbars/Editor/EditorToolBar.h
  Toolbars/Main/MainToolBar.h
  Toolbars/Segmentation/DefaultVOIAction.h
  Toolbars/Segmentation/SegmentationToolBar.h
  Toolbars/Segmentation/ThresholdAction.h
  Toolbars/VOI/VolumeOfInterest.h
  Toolbars/Zoom/ZoomToolBar.h
  Tools/Brushes/Brush.h
  Tools/PlanarSplit/PlanarSplitTool.h
  Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h
  Tools/SegmentationRemover/SegmentationRemover.h
  Tools/VOI/RectangularVOI.h
  Views/DefaultEspinaView.h
  Dialogs/Connectomics/ConnectomicsDialog.h
  Dialogs/ChannelInspector/ChannelInspector.h
  Undo/BrushUndoCommand.h
  )

# Create Interface files
qt4_wrap_ui (ESPINA_UI
  Dialogs/SegmentationInspector/SegmentationInspector.ui
  Dialogs/TabularReport/QueryView.ui
  Dialogs/AboutDialog.ui
  Dialogs/SettingsDialog.ui
  Docks/ChannelExplorer/ChannelExplorer.ui
  Docks/SegmentationExplorer/SegmentationExplorer.ui
  Docks/TabularReport/DataView.ui
  FilterInspectors/ContourSource/ContourInspectorWidget.ui
  FilterInspectors/SeedGrowSegmentation/SGSFilterInspectorWidget.ui
  Settings/GeneralSettingsPanel.ui
  Settings/Editor/EditorToolBarSettingsPanel.ui
  Settings/SeedGrowSegmentation/SettingsPanel.ui
  Settings/SliceView/SliceViewSettingsPanel.ui
  Settings/VOI/RectangularVOISettings.ui
  Settings/VolumeView/VolumeViewSettingsPanel.ui
  Dialogs/Connectomics/ConnectomicsDialog.ui
  Dialogs/ChannelInspector/ChannelInspector.ui
  )

set (ESPINA_SOURCES
  ${ESPINA_RCC}
  ${ESPINA_MOC}
  ${ESPINA_UI}
  main.cpp
  EspinaMainWindow.cpp
  RecentDocuments.cpp
  Dialogs/SegmentationInspector/SegmentationInspector.cpp
  Dialogs/TabularReport/QueryView.cpp #TODO 2012-11-27 Cambiar nombre de la clase a TabularReport
  Dialogs/AboutDialog.cpp
  Dialogs/SettingsDialog.cpp #TODO 2012-11-27 Renombrar General Settings Dialog
  Docks/ChannelExplorer/ChannelExplorer.cpp
  Docks/FilterInspector/FilterInspector.cpp # TODO 2012-11-27 Renombrar (cuidadin con el Filter::FilterInspector)
  Docks/SegmentationExplorer/LayoutComposition.cpp
  Docks/SegmentationExplorer/LayoutSample.cpp
  Docks/SegmentationExplorer/LayoutTaxonomy.cpp
  Docks/SegmentationExplorer/SegmentationExplorerLayout.cpp
  Docks/SegmentationExplorer/SegmentationDelegate.cpp
  Docks/SegmentationExplorer/SegmentationExplorer.cpp
  Docks/TabularReport/DataView.cpp
  Docks/TabularReport/DataViewPanel.cpp
  FilterInspectors/CODE/CODEFilterInspector.cpp
  FilterInspectors/ContourSource/ContourInspector.cpp
  FilterInspectors/ContourSource/ContourInspectorWidget.cpp
  FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.cpp
  Menus/ColorEngineMenu.cpp
  Settings/GeneralSettings.cpp
  Settings/Editor/SettingsPanel.cpp
  Settings/SeedGrowSegmentation/SettingsPanel.cpp
  Settings/SliceView/SliceViewSettingsPanel.cpp
  Settings/VOI/SettingsPanel.cpp
  Settings/VolumeView/VolumeViewSettingsPanel.cpp
  Toolbars/Composition/CompositionToolBar.cpp
  Toolbars/Editor/EditorToolBar.cpp
  Toolbars/Editor/Settings.cpp
  Toolbars/Main/MainToolBar.cpp
  Toolbars/Segmentation/DefaultVOIAction.cpp
  Toolbars/Segmentation/SegmentationToolBar.cpp
  Toolbars/Segmentation/SeedGrowSegmentationSettings.cpp
  Toolbars/Segmentation/ThresholdAction.cpp
  Toolbars/VOI/Settings.cpp
  Toolbars/VOI/VolumeOfInterest.cpp
  Toolbars/Zoom/ZoomToolBar.cpp
  Tools/Brushes/Brush.cpp
  Tools/Brushes/CircularBrush.cpp
  Tools/Brushes/SphericalBrush.cpp
  Tools/Contour/FilledContour.cpp
  Tools/PlanarSplit/PlanarSplitTool.cpp
  Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.cpp
  Tools/SegmentationRemover/SegmentationRemover.cpp
  Tools/VOI/RectangularVOI.cpp
  Tools/Zoom/ZoomTool.cpp
  Undo/BrushUndoCommand.cpp
  Undo/CompositionCommand.cpp
  Undo/ImageLogicCommand.cpp
  Undo/MoveTaxonomiesCommand.cpp
  Undo/SeedGrowSegmentationCommand.cpp
  Undo/SplitUndoCommand.cpp
  Undo/RemoveTaxonomyElementCommand.cpp
  Views/DefaultEspinaView.cpp
  Dialogs/Connectomics/ConnectomicsDialog.cpp
  Tools/Measure/MeasureTool.cpp
  Dialogs/ChannelInspector/ChannelInspector.cpp
  )

set (ESPINA_LIBRARIES
  EspinaCore
  EspinaFilters
  EspinaGUI
  EspinaUndo
  QVTK
  QtTest # Required by ModelTest
)


add_executable(espina ${ESPINA_SOURCES})
target_link_libraries (espina ${ESPINA_LIBRARIES})

#install (
#  TARGETS espina
#  RUNTIME
#  DESTINATION bin
##  COMPONENT espina
#  )
#
#
#add_subdirectory (plugins)
