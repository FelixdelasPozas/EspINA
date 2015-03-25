include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}     # Generated EspinaConfig.h
  ${CMAKE_BINARY_DIR}/GUI # Generated UI files
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(ESPINA_RCC
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (ESPINA_MOC
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h
  Dialogs/ChannelInspector/ChannelInspector.h
  Dialogs/CheckAnalysis/CheckAnalysis.h
  Dialogs/SegmentationInspector/SegmentationInspector.h
  Dialogs/Settings/GeneralSettingsDialog.h
  Docks/ChannelExplorer/ChannelExplorer.h
  Docks/SegmentationExplorer/Layouts/ClassificationLayout.h
  Docks/SegmentationExplorer/SegmentationExplorer.h
  Docks/SegmentationExplorer/SegmentationExplorerLayout.h
  Docks/SegmentationHistory/DefaultHistory.h
  Docks/SegmentationHistory/EmptyHistory.h
  Docks/SegmentationHistory/HistoryDock.h
  EspinaMainWindow.h
  Menus/CamerasMenu.h
  Menus/ColorEngineMenu.h
  Menus/DefaultContextualMenu.h
  Menus/RenderersMenu.h
  Settings/ROI/ROISettingsPanel.h
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.h
  Settings/View2D/View2DRenderersPanel.h
  Settings/View2D/View2DSettingsPanel.h
  Settings/View3D/View3DSettingsPanel.h
  ToolGroups/Explore/ResetZoom.h
  ToolGroups/Explore/ToggleCrosshairVisibility.h
  ToolGroups/Explore/ZoomAreaTool.h
  ToolGroups/Refine/CODEHistory.h
  ToolGroups/Refine/CODEHistoryWidget.h
  ToolGroups/Refine/CODETool.h
  ToolGroups/Refine/ManualEditionTool.h
  ToolGroups/Refine/MorphologicalEditionTool.h
  ToolGroups/Refine/MorphologicalEditionTool.h
  ToolGroups/Refine/RefineToolGroup.h
  ToolGroups/Refine/SplitTool.h
  ToolGroups/Restrict/CleanROITool.h
  ToolGroups/Restrict/OrthogonalROITool.h
  ToolGroups/Restrict/RestrictToolGroup.h
  ToolGroups/Segmentate/SeedGrowSegmentation/CustomROIWidget.h
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationHistory.h
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationHistoryWidget.h
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationSettings.h
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationTool.h
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedThreshold.h
  ToolGroups/Segmentate/Skeleton/SkeletonTool.h
  ToolGroups/Segmentate/Skeleton/SkeletonToolGroup.h
  ToolGroups/Segmentate/Skeleton/SkeletonToolStatusAction.h
  ToolGroups/Visualize/Representations/CrosshairsSwitch2D.h
  ToolGroups/Visualize/Representations/SegmentationSlice/SegmentationSliceSwitch.h
  ToolGroups/Visualize/Representations/SegmentationVolumetric/VolumetricManager.h
  ToolGroups/Visualize/VisualizeToolGroup.h
  ToolGroups/Analyze/AnalyzeToolGroup.h
  ToolGroups/ToolGroup.h
  Tools/Measure/MeasureTool.h
  Tools/Ruler/RulerTool.h
  Undo/DrawUndoCommand.h
  Views/DefaultView.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

# Create Interface files
qt4_wrap_ui (ESPINA_UI
  Dialogs/About/AboutDialog.ui
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.ui
  Dialogs/ChannelInspector/ChannelInspector.ui
  Dialogs/CheckAnalysis/CheckAnalysisDialog.ui
  Dialogs/ProblemList/ProblemListDialog.ui
  Dialogs/SegmentationInspector/SegmentationInspector.ui
  Dialogs/Settings/GeneralSettingsDialog.ui
  Docks/ChannelExplorer/ChannelExplorer.ui
  Docks/SegmentationExplorer/SegmentationExplorer.ui
  Docks/SegmentationHistory/DefaultHistory.ui
  Docks/SegmentationHistory/EmptyHistory.ui
  Settings/GeneralSettings/GeneralSettingsPanel.ui
  Settings/ROI/OrthogonalROISettings.ui
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.ui
  Settings/View2D/View2DSettingsPanel.ui
  Settings/View3D/View3DSettingsPanel.ui
  ToolGroups/Refine/CODEHistoryWidget.ui
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationHistoryWidget.ui
  )

if (METADONA_FOUND)
  set (METADONA_SOURCES
    Settings/MetaData/MetaDataSettingsPanel.cpp
    )
endif (METADONA_FOUND)

set (ESPINA_SOURCES
  ${ESPINA_RCC}
  ${ESPINA_MOC}
  ${ESPINA_UI}
  ${METADONA_SOURCES}
  main.cpp
  Dialogs/About/AboutDialog.cpp
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.cpp
  Dialogs/ChannelInspector/ChannelInspector.cpp
  Dialogs/CheckAnalysis/CheckAnalysis.cpp
  Dialogs/ProblemList/ProblemListDialog.cpp
  Dialogs/SegmentationInspector/SegmentationInspector.cpp
  Dialogs/Settings/GeneralSettingsDialog.cpp
  Dialogs/RawInformation/RawInformationDialog.cpp
  Docks/ChannelExplorer/ChannelExplorer.cpp
  Docks/SegmentationExplorer/Layouts/ClassificationLayout.cpp
  Docks/SegmentationExplorer/SegmentationExplorer.cpp
  Docks/SegmentationExplorer/SegmentationExplorerLayout.cpp
  Docks/SegmentationHistory/DefaultHistory.cpp
  Docks/SegmentationHistory/EmptyHistory.cpp
  Docks/SegmentationHistory/HistoryDock.cpp
  EspinaErrorHandler.cpp
  EspinaMainWindow.cpp
  IO/SegFileReader.cpp
  Menus/CamerasMenu.cpp
  Menus/ColorEngineMenu.cpp
  Menus/DefaultContextualMenu.cpp
  Menus/RenderersMenu.cpp
  RecentDocuments.cpp
  Settings/DefaultView/DefaultViewSettingsPanel.cpp
  Settings/GeneralSettings/GeneralSettings.cpp
  Settings/GeneralSettings/GeneralSettingsPanel.cpp
  Settings/ROI/ROISettings.cpp
  Settings/ROI/ROISettingsPanel.cpp
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.cpp
  Settings/View2D/View2DRenderersPanel.cpp
  Settings/View2D/View2DSettingsPanel.cpp
  Settings/View3D/View3DSettingsPanel.cpp
  ToolGroups/Visualize/Representations/ChannelLines/ChannelLinesPipeline.cpp
  ToolGroups/Visualize/Representations/ChannelSlice/ChannelSlicePipeline.cpp
  ToolGroups/Visualize/Representations/ChannelSlice/ChannelSliceRepresentationFactory.cpp
  ToolGroups/Visualize/Representations/CrosshairsSwitch2D.cpp
  ToolGroups/Visualize/Representations/SegmentationSlice/SegmentationSlicePipeline.cpp
  ToolGroups/Visualize/Representations/SegmentationSlice/SegmentationSliceRepresentationFactory.cpp
  ToolGroups/Visualize/Representations/SegmentationSlice/SegmentationSliceSettings.cpp
  ToolGroups/Visualize/Representations/SegmentationSlice/SegmentationSliceSwitch.cpp
  ToolGroups/Visualize/Representations/SegmentationVolumetric/SegmentationVolumetricPipeline.cpp
  ToolGroups/Visualize/Representations/SegmentationVolumetric/VolumetricManager.cpp
  ToolGroups/Explore/ResetZoom.cpp
  ToolGroups/Explore/ToggleCrosshairVisibility.cpp
  ToolGroups/Visualize/VisualizeToolGroup.cpp
  ToolGroups/Explore/ZoomAreaTool.cpp
  ToolGroups/Restrict/CleanROITool.cpp
  ToolGroups/Restrict/OrthogonalROITool.cpp
  ToolGroups/Restrict/RestrictToolGroup.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/CustomROIWidget.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationHistory.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationHistoryWidget.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationSettings.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedGrowSegmentationTool.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SeedThreshold.cpp
  ToolGroups/Segmentate/SeedGrowSegmentation/SegmentationTools.cpp
  ToolGroups/Segmentate/Skeleton/SkeletonTool.cpp
  ToolGroups/Segmentate/Skeleton/SkeletonToolGroup.cpp
  ToolGroups/Segmentate/Skeleton/SkeletonToolStatusAction.cpp
  ToolGroups/Refine/CODEHistory.cpp
  ToolGroups/Refine/CODEHistoryWidget.cpp
  ToolGroups/Refine/CODETool.cpp
  ToolGroups/Refine/RefineToolGroup.cpp
  ToolGroups/Refine/ManualEditionTool.cpp
  ToolGroups/Refine/MorphologicalEditionTool.cpp
  ToolGroups/Refine/SliceEditionPipeline.cpp
  ToolGroups/Refine/SplitTool.cpp
  ToolGroups/Analyze/AnalyzeToolGroup.cpp
  ToolGroups/ToolGroup.cpp
  Tools/Measure/MeasureTool.cpp
  Tools/Ruler/RulerTool.cpp
  Undo/DrawUndoCommand.cpp
  Undo/ModifySkeletonCommand.cpp
  Undo/ROIUndoCommand.cpp
  Undo/RenameSegmentationsCommand.cpp
  Views/DefaultView.cpp
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
  ${METADONA_LIBRARIES}
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

install(TARGETS espina
  # IMPORTANT: Add the espina executable to the "export-set"
  EXPORT ESPINATargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  )