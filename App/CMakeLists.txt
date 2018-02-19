find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

find_package(XLSLIB REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}     # Generated EspinaConfig.h
  ${CMAKE_BINARY_DIR}/GUI # Generated UI files
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  ${XLSLIB_INCLUDE_DIRS}
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(ESPINA_RCC
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (ESPINA_MOC
  AutoSave.h
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.h
  Dialogs/AdjacencyMatrix/AdjacencyMatrixTabularReport.h
  Dialogs/ConnectionCount/ConnectionCountDialog.h
  Dialogs/CustomFileOpenDialog/CustomFileDialog.h
  Dialogs/CustomFileOpenDialog/OptionsPanel.h
  Dialogs/StackInspector/StackInspector.h
  Dialogs/Dialog3D/Dialog3D.h
  Dialogs/DistanceInformation/DistanceInformationOptionsDialog.h
  Dialogs/DistanceInformation/DistanceInformationTabularReport.h
  Dialogs/HueSelector/HueSelector.h
  Dialogs/IssueList/CheckAnalysis.h
  Dialogs/SegmentationInspector/SegmentationInspector.h
  Dialogs/Settings/GeneralSettingsDialog.h
  Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.h
  Dialogs/SkeletonInspector/SkeletonInspector.h
  Dialogs/SkeletonInspector/SkeletonInspectorTreeModel.h
  Panels/StackExplorer/StackExplorer.h
  Panels/SegmentationExplorer/Layouts/ClassificationLayout.h
  Panels/SegmentationExplorer/SegmentationExplorer.h
  Panels/SegmentationExplorer/SegmentationExplorerLayout.h
  Panels/SegmentationProperties/NoFilterRefiner.h
  Panels/SegmentationProperties/SegmentationProperties.h
  EspinaMainWindow.h
  Menus/ColorEngineMenu.h
  Menus/DefaultContextualMenu.h
  Settings/GeneralSettings/GeneralSettingsPanel.h
  Settings/ROI/ROISettingsPanel.h
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.h
  ToolGroups/Analyze/ConnectionCount/ConnectionCountTool.h
  ToolGroups/Analyze/Reports/ReportSelectorDialog.h
  ToolGroups/Analyze/Reports/DistanceInformationReport.h
  ToolGroups/Analyze/Reports/ReportsTool.h
  ToolGroups/Analyze/AnalyzeToolGroup.h
  ToolGroups/Analyze/SkeletonInspector/SkeletonInspectorTool.h
  ToolGroups/Explore/ResetViewTool.h
  ToolGroups/Explore/PositionMarksTool.h
  ToolGroups/Explore/ZoomRegionTool.h
  ToolGroups/Edit/CODERefineWidget.h
  ToolGroups/Edit/CODERefiner.h
  ToolGroups/Edit/CODETool.h
  ToolGroups/Edit/FillHoles2DTool.h
  ToolGroups/Edit/FillHolesTool.h
  ToolGroups/Edit/ImageLogicTool.h
  ToolGroups/Edit/ManualEditionTool.h
  ToolGroups/Edit/EditToolGroup.h
  ToolGroups/Edit/SplitTool.h
  ToolGroups/Edit/SkeletonEditionTool.h
  ToolGroups/Session/FileOpenTool.h
  ToolGroups/Session/FileSaveTool.h
  ToolGroups/Session/UndoRedoTools.h
  ToolGroups/Restrict/DeleteROITool.h
  ToolGroups/Restrict/FreehandROITool.h
  ToolGroups/Restrict/OrthogonalROITool.h
  ToolGroups/Restrict/RestrictToolGroup.h
  ToolGroups/Segment/Manual/ManualSegmentTool.h
  ToolGroups/Segment/SeedGrowSegmentation/CustomROIWidget.h
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationRefineWidget.h
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.h
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationTool.h
  ToolGroups/Segment/SeedGrowSegmentation/SeedTemporalPrototype.h
  ToolGroups/Segment/SeedGrowSegmentation/SeedThreshold.h
  ToolGroups/Segment/Skeleton/SkeletonCreationTool.h
  ToolGroups/Segment/Skeleton/SkeletonToolsEventHandler.h
  ToolGroups/Segment/Skeleton/ConnectionPointsTemporalRepresentation2D.h
  ToolGroups/Segment/Skeleton/SkeletonToolWidget2D.h
  ToolGroups/ToolGroup.h
  ToolGroups/Visualize/ColorEngines/InformationColorEngineSwitch.h
  ToolGroups/Visualize/FullscreenTool.h
  ToolGroups/Visualize/Representations/Switches/SegmentationConnectionsSwitch.h
  ToolGroups/Visualize/Representations/Switches/SegmentationContourSwitch.h
  ToolGroups/Visualize/Representations/Switches/SegmentationMeshSwitch.h
  ToolGroups/Visualize/Representations/Switches/SegmentationVolumetricSwitch.h
  ToolGroups/Visualize/Representations/Switches/SegmentationSliceSwitch.h
  ToolGroups/Visualize/Representations/Switches/SegmentationSkeletonSwitch.h
  ToolGroups/Analyze/MeasureLength/MeasureLengthTool.h
  ToolGroups/Analyze/SelectionMeasure/SelectionMeasureTool.h
  Undo/DrawUndoCommand.h
  Views/DefaultView.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

# Create Interface files
qt4_wrap_ui (ESPINA_UI
  Dialogs/About/AboutDialog.ui
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.ui
  Dialogs/ConnectionCount/ConnectionCountDialog.ui
  Dialogs/CustomFileOpenDialog/OptionsPanel.ui
  Dialogs/StackInspector/StackInspector.ui
  Dialogs/Dialog3D/Dialog3D.ui
  Dialogs/DistanceInformation/DistanceInformationOptionsDialog.ui
  Dialogs/HueSelector/HueSelectorDialog.ui
  Dialogs/IssueList/IssueListDialog.ui
  Dialogs/SegmentationInspector/SegmentationInspector.ui
  Dialogs/Settings/GeneralSettingsDialog.ui
  Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.ui
  Dialogs/SkeletonInspector/SkeletonInspector.ui
  Panels/StackExplorer/StackExplorer.ui
  Panels/SegmentationExplorer/SegmentationExplorer.ui
  Panels/SegmentationProperties/IssueProperty.ui
  Panels/SegmentationProperties/NoFilterRefiner.ui
  Panels/SegmentationProperties/SegmentationProperties.ui
  Settings/GeneralSettings/GeneralSettingsPanel.ui
  Settings/ROI/OrthogonalROISettings.ui
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.ui
  ToolGroups/Analyze/Reports/ReportSelectorDialog.ui
  ToolGroups/Edit/CODERefineWidget.ui
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationRefineWidget.ui
  ${ESPINA_INCLUDE}/Support/Widgets/TabularReportEntry.ui
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
  AutoSave.cpp
  Dialogs/About/AboutDialog.cpp
  Dialogs/AdaptiveEdges/AdaptiveEdgesDialog.cpp
  Dialogs/AdjacencyMatrix/AdjacencyMatrixDialog.cpp
  Dialogs/AdjacencyMatrix/AdjacencyMatrixTabularReport.cpp
  Dialogs/ConnectionCount/ConnectionCountDialog.cpp
  Dialogs/CustomFileOpenDialog/CustomFileDialog.cpp
  Dialogs/CustomFileOpenDialog/OptionsPanel.cpp
  Dialogs/StackInspector/StackInspector.cpp
  Dialogs/HueSelector/HueSelector.cpp
  Dialogs/IssueList/CheckAnalysis.cpp
  Dialogs/IssueList/IssueListDialog.cpp
  Dialogs/DistanceInformation/DistanceInformationDialog.cpp
  Dialogs/DistanceInformation/DistanceInformationOptionsDialog.cpp
  Dialogs/DistanceInformation/DistanceInformationTabularReport.cpp
  Dialogs/RawInformation/RawInformationDialog.cpp
  Dialogs/SegmentationInspector/SegmentationInspector.cpp
  Dialogs/Settings/GeneralSettingsDialog.cpp
  Dialogs/Dialog3D/Dialog3D.cpp
  Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.cpp
  Dialogs/SkeletonInspector/SkeletonInspector.cpp
  Dialogs/SkeletonInspector/SkeletonInspectorTreeModel.cpp
  EspinaErrorHandler.cpp
  EspinaMainWindow.cpp
  IO/SegFileReader.cpp
  Menus/ColorEngineMenu.cpp
  Menus/DefaultContextualMenu.cpp
  Panels/SegmentationExplorer/Layouts/ClassificationLayout.cpp
  Panels/SegmentationExplorer/SegmentationExplorer.cpp
  Panels/SegmentationExplorer/SegmentationExplorerLayout.cpp
  Panels/SegmentationProperties/IssueProperty.cpp
  Panels/SegmentationProperties/NoFilterRefiner.cpp
  Panels/SegmentationProperties/SegmentationProperties.cpp
  Panels/StackExplorer/StackExplorer.cpp
  RecentDocuments.cpp
  Settings/GeneralSettings/GeneralSettingsPanel.cpp
  Settings/ROI/ROISettings.cpp
  Settings/ROI/ROISettingsPanel.cpp
  Settings/SeedGrowSegmentation/SeedGrowSegmentationSettingsPanel.cpp
  Settings/Utils.cpp
  ToolGroups/Analyze/AnalyzeToolGroup.cpp
  ToolGroups/Analyze/ConnectionCount/ConnectionCountTool.cpp
  ToolGroups/Analyze/MeasureLength/MeasureLengthTool.cpp
  ToolGroups/Analyze/Reports/DistanceInformationReport.cpp
  ToolGroups/Analyze/Reports/RawInformationReport.cpp
  ToolGroups/Analyze/Reports/ReportSelectorDialog.cpp
  ToolGroups/Analyze/Reports/ReportsTool.cpp
  ToolGroups/Analyze/Reports/AdjacencyMatrixReport.cpp
  ToolGroups/Analyze/SelectionMeasure/SelectionMeasureTool.cpp
  ToolGroups/Analyze/SkeletonInspector/SkeletonInspectorTool.cpp
  ToolGroups/Edit/CODERefineWidget.cpp
  ToolGroups/Edit/CODERefiner.cpp
  ToolGroups/Edit/CODETool.cpp
  ToolGroups/Edit/EditToolGroup.cpp
  ToolGroups/Edit/FillHoles2DTool.cpp
  ToolGroups/Edit/FillHolesTool.cpp
  ToolGroups/Edit/ImageLogicTool.cpp
  ToolGroups/Edit/ManualEditionTool.cpp
  ToolGroups/Edit/SliceEditionPipeline.cpp
  ToolGroups/Edit/SplitTool.cpp
  ToolGroups/Edit/SkeletonEditionTool.cpp
  ToolGroups/Explore/PositionMarksTool.cpp
  ToolGroups/Explore/ResetViewTool.cpp
  ToolGroups/Explore/ZoomRegionTool.cpp
  ToolGroups/Restrict/DeleteROITool.cpp
  ToolGroups/Restrict/FreehandROITool.cpp
  ToolGroups/Restrict/OrthogonalROITool.cpp
  ToolGroups/Restrict/RestrictToolGroup.cpp
  ToolGroups/Segment/Manual/ManualSegmentTool.cpp
  ToolGroups/Segment/SeedGrowSegmentation/CustomROIWidget.cpp
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationRefineWidget.cpp
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationSettings.cpp
  ToolGroups/Segment/SeedGrowSegmentation/SeedGrowSegmentationTool.cpp
  ToolGroups/Segment/SeedGrowSegmentation/SeedTemporalPrototype.cpp
  ToolGroups/Segment/SeedGrowSegmentation/SeedThreshold.cpp
  ToolGroups/Segment/Skeleton/SkeletonCreationTool.cpp
  ToolGroups/Segment/Skeleton/SkeletonToolsUtils.cpp
  ToolGroups/Segment/Skeleton/SkeletonToolsEventHandler.cpp
  ToolGroups/Segment/Skeleton/ConnectionPointsTemporalRepresentation2D.cpp
  ToolGroups/Segment/Skeleton/SkeletonToolWidget2D.cpp
  ToolGroups/Session/ChunkReporter.cpp
  ToolGroups/Session/FileOpenTool.cpp
  ToolGroups/Session/FileSaveTool.cpp
  ToolGroups/Session/UndoRedoTools.cpp
  ToolGroups/ToolGroup.cpp
  ToolGroups/Visualize/ColorEngines/InformationColorEngineSwitch.cpp
  ToolGroups/Visualize/FullscreenTool.cpp
  ToolGroups/Visualize/GenericTogglableTool.cpp
  ToolGroups/Visualize/Representations/StackRepresentationFactory.cpp
  ToolGroups/Visualize/Representations/CrosshairRepresentationFactory.cpp
  ToolGroups/Visualize/Representations/SegmentationRepresentationFactory.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationConnectionsSwitch.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationContourSwitch.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationMeshSwitch.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationVolumetricSwitch.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationSliceSwitch.cpp
  ToolGroups/Visualize/Representations/Switches/SegmentationSkeletonSwitch.cpp
  Undo/ChangeCategoryColorCommand.cpp
  Undo/DragChannelsCommand.cpp
  Undo/DrawUndoCommand.cpp
  Undo/ModifySkeletonCommand.cpp
  Undo/ROIUndoCommand.cpp
  Undo/RenameSegmentationsCommand.cpp
  Undo/RemoveChannel.cpp
  Utils/TagUtils.cpp
  Views/DefaultView.cpp
  main.cpp
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

install(TARGETS espina
  # IMPORTANT: Add the espina executable to the "export-set"
  EXPORT ESPINATargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  )