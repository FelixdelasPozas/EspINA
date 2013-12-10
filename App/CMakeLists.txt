#find_package(XLSLIB REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR} # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
#  ${XLSLIB_INCLUDE_DIRS}
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(ESPINA_RCC
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (ESPINA_MOC
  EspinaMainWindow.h
#   Dialogs/SegmentationInspector/SegmentationInspector.h
#   Dialogs/TabularReport/TabularReport.h
#   Dialogs/TabularReport/TabularReportEntry.h
#   Dialogs/GeneralSettingsDialog.h
  Docks/ChannelExplorer/ChannelExplorer.h
  Docks/SegmentationExplorer/SegmentationExplorer.h
  Docks/SegmentationExplorer/Layouts/ClassificationLayout.h
#   Docks/FilterInspectorDock/FilterInspectorDock.h
  Docks/SegmentationExplorer/SegmentationExplorerLayout.h
#   Docks/SegmentationExplorer/LayoutLocation.h
#   Docks/SegmentationExplorer/LayoutComposition.h
#   FilterInspectors/CODE/CODEFilterInspector.h
#   FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h
  Menus/ColorEngineMenu.h
#   Menus/SegmentationContextualMenu.h
#   Settings/Editor/SettingsPanel.h
#   Settings/SeedGrowSegmentation/SettingsPanel.h
#   Settings/SliceView/SliceViewSettingsPanel.h
#   Settings/VOI/SettingsPanel.h
#   Toolbars/Composition/CompositionToolBar.h
#   Toolbars/Editor/EditorToolBar.h
#   Toolbars/Main/MainToolBar.h
#   Toolbars/Segmentation/SegmentationToolBar.h
#   Toolbars/VOI/VolumeOfInterest.h
  ToolGroups/ViewState/ViewTools.h
  ToolGroups/ViewState/ToggleSegmentationsVisibility.h
  ToolGroups/ViewState/ToggleCrosshairVisibility.h
  ToolGroups/Segmentation/ApplyROI.h
  ToolGroups/Segmentation/SeedThreshold.h
  ToolGroups/Segmentation/SeedGrowSegmentationTool.h
  ToolGroups/Segmentation/SegmentationTools.h
  ToolGroups/Editor/EditionTools.h
  ToolGroups/Editor/SplitTool.h
  ToolGroups/Editor/MorphologicalEditionTool.h
  ToolGroups/Editor/ManualEditionTool.h
  ToolGroups/Editor/MorphologicalEditionTool.h
  Tools/Brushes/CircularBrushSelector.h
  Tools/Brushes/SphericalBrushSelector.h
#   Tools/Contour/FilledContour.h
#   Tools/PlanarSplit/PlanarSplitTool.h
#   Tools/Measure/MeasureTool.h
#   Tools/Ruler/RulerTool.h
#   Tools/SegmentationRemover/SegmentationRemover.h
#   Tools/VOI/RectangularVOI.h
  Views/DefaultView.h
#   Dialogs/Connectomics/ConnectomicsDialog.h
#   Dialogs/ChannelInspector/ChannelInspector.h
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h
  Undo/BrushUndoCommand.h
#   Tools/TubularSegmentation/TubularTool.h
#   FilterInspectors/TubularSegmentation/TubularFilterInspector.h
#   FilterInspectors/TubularSegmentation/NodesInformationDialog.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

# Create Interface files
qt4_wrap_ui (ESPINA_UI
#   Dialogs/SegmentationInspector/SegmentationInspector.ui
#   Dialogs/TabularReport/TabularReport.ui
  Dialogs/AboutDialog.ui
#   Dialogs/GeneralSettingsDialog.ui
  Docks/ChannelExplorer/ChannelExplorer.ui
  Docks/SegmentationExplorer/SegmentationExplorer.ui
#   Docks/TabularReport/TabularReport.ui
#   FilterInspectors/SeedGrowSegmentation/SGSFilterInspectorWidget.ui
#   Settings/GeneralSettingsPanel.ui
#   Settings/Editor/EditorSettingsPanel.ui
#   Settings/SeedGrowSegmentation/SeedGrowSettingsPanel.ui
#   Settings/SliceView/SliceViewSettingsPanel.ui
#   Settings/VOI/RectangularVOISettings.ui
#   Settings/VolumeView/VolumeViewSettingsPanel.ui
#   Dialogs/Connectomics/ConnectomicsDialog.ui
#   Dialogs/ChannelInspector/ChannelInspector.ui
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.ui
#   FilterInspectors/TubularSegmentation/TubularFilterInspector.ui
#   FilterInspectors/TubularSegmentation/NodesInformationDialog.ui
  )

set (ESPINA_SOURCES 
  ${ESPINA_RCC}
  ${ESPINA_MOC}
  ${ESPINA_UI}
  main.cpp
  EspinaErrorHandler.cpp 
  EspinaMainWindow.cpp
  IO/ChannelReader.cpp
  IO/SegFileReader.cpp
  RecentDocuments.cpp
#   Dialogs/SegmentationInspector/SegmentationInspector.cpp
#   Dialogs/TabularReport/TabularReport.cpp
#   Dialogs/TabularReport/RawInformationDialog.cpp
#   Dialogs/TabularReport/TabularReportEntry.cpp 
  Dialogs/AboutDialog.cpp
#   Dialogs/GeneralSettingsDialog.cpp
  Docks/ChannelExplorer/ChannelExplorer.cpp
  Docks/SegmentationExplorer/SegmentationExplorer.cpp
  Docks/SegmentationExplorer/Layouts/ClassificationLayout.cpp
  Docks/SegmentationExplorer/SegmentationExplorerLayout.cpp
#   Docks/FilterInspectorDock/FilterInspectorDock.cpp
#   Docks/SegmentationExplorer/LayoutComposition.cpp
#   Docks/SegmentationExplorer/LayoutLocation.cpp
#   FilterInspectors/CODE/CODEFilterInspector.cpp
#   FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.cpp
  Menus/ColorEngineMenu.cpp
#   Menus/SegmentationContextualMenu.cpp
  Settings/GeneralSettings.cpp
#   Settings/Editor/SettingsPanel.cpp
#   Settings/SeedGrowSegmentation/SettingsPanel.cpp
#   Settings/SliceView/SliceViewSettingsPanel.cpp
#   Settings/VOI/SettingsPanel.cpp
#   Settings/VolumeView/VolumeViewSettingsPanel.cpp
#   Toolbars/Composition/CompositionToolBar.cpp
#   Toolbars/Editor/EditorToolBar.cpp
#   Toolbars/Editor/Settings.cpp
#   Toolbars/Main/MainToolBar.cpp
#   Toolbars/Segmentation/SeedGrowSegmentationSettings.cpp
#   Toolbars/VOI/Settings.cpp
#   Toolbars/VOI/VolumeOfInterest.cpp
  ToolGroups/Segmentation/ApplyROI.cpp
  ToolGroups/Segmentation/SegmentationTools.cpp
  ToolGroups/Segmentation/SeedGrowSegmentationTool.cpp
  ToolGroups/Segmentation/SeedThreshold.cpp
  ToolGroups/ViewState/ResetZoom.cpp
  ToolGroups/ViewState/ToggleSegmentationsVisibility.cpp 
  ToolGroups/ViewState/ToggleCrosshairVisibility.cpp
  ToolGroups/ViewState/ViewTools.cpp
  ToolGroups/ViewState/ZoomArea.cpp
  ToolGroups/Editor/EditionTools.cpp
  ToolGroups/Editor/SplitTool.cpp
  ToolGroups/Editor/MorphologicalEditionTool.cpp
  ToolGroups/Editor/ManualEditionTool.cpp
  Tools/Brushes/CircularBrushSelector.cpp
  Tools/Brushes/SphericalBrushSelector.cpp
#   Tools/Contour/FilledContour.cpp
#   Tools/Measure/MeasureTool.cpp
#   Tools/PlanarSplit/PlanarSplitTool.cpp
#   Tools/Ruler/RulerTool.cpp
#   Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.cpp
#   Tools/SegmentationRemover/SegmentationRemover.cpp
#   Tools/VOI/RectangularVOI.cpp
#   Tools/Zoom/ZoomTool.cpp
#   Undo/AtomicModelOperations.cpp 
  Undo/BrushUndoCommand.cpp
#   Undo/CompositionCommand.cpp
#   Undo/ImageLogicCommand.cpp
#   Undo/SeedGrowSegmentationCommand.cpp
#   Undo/SplitUndoCommand.cpp
#   Undo/UndoableEspinaModel.cpp
#   Undo/UnloadChannelCommand.cpp
#   Undo/UnloadSampleCommand.cpp
#   Undo/VolumeSnapshotCommand.cpp
#   Undo/ContourUndoCommand.cpp
  Views/DefaultView.cpp
#   Dialogs/Connectomics/ConnectomicsDialog.cpp
#   Dialogs/ChannelInspector/ChannelInspector.cpp
#   Tools/TubularSegmentation/TubularTool.cpp
#   FilterInspectors/TubularSegmentation/TubularFilterInspector.cpp
#   FilterInspectors/TubularSegmentation/NodesInformationDialog.cpp
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.cpp
  )

set (ESPINA_LIBRARIES
  EspinaCore
  EspinaExtensions
  EspinaFilters
  EspinaGUI
  EspinaSupport
  EspinaUndo
)

set (EXTERNAL_LIBRARIES
  ${XLSLIB_LIBRARIES}
  )

if(DEFINED MINGW OR DEFINED MSVC)
  set (EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} ${QT_LIBRARIES} ${QT_QTTEST_LIBRARY})
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/winexeprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/winexeprops.rc")
    SET(ESPINA_SOURCES ${ESPINA_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/winexeprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
else (DEFINED MINGW OR DEFINED MSVC)
  set (EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} QtTest )
endif (DEFINED MINGW OR DEFINED MSVC)

add_executable(espina ${ESPINA_SOURCES})
target_link_libraries (espina ${ESPINA_LIBRARIES} ${EXTERNAL_LIBRARIES})

#install (
#  TARGETS espina
#  RUNTIME
#  DESTINATION bin
##  COMPONENT espina
#  )
#
#
#add_subdirectory (plugins)