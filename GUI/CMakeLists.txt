find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

include (GenerateExportHeader)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt5_wrap_cpp(GUI_MOC
  ColorEngines/CategoryColorEngine.h
  ColorEngines/ColorEngine.h
  ColorEngines/MultiColorEngine.h
  Dialogs/ImageResolutionDialog/ImageResolutionDialog.h
  Dialogs/RangeDefinitionDialog/ColorEngineRangeDefinitionDialog.h
  Dialogs/RangeLimitsDialog/ColorEngineRangeLimitsDialog.h
  Model/ItemAdapter.h
  Model/ModelAdapter.h
  Model/ViewItemAdapter.h
  Model/Proxies/ChannelProxy.h
  Model/Proxies/ClassificationProxy.h
  Model/Proxies/InformationProxy.h
  Model/Proxies/LocationProxy.h
  Model/ViewItemAdapter.h
  Model/Utils/DBVH.h
  Representations/ModelSources.h
  Representations/PipelineSources.h
  Representations/RepresentationManager.h
  Representations/Managers/ConnectionsManager.h
  Representations/Managers/PoolManager.h
  Representations/Managers/PassiveActorManager.h
  Representations/RepresentationPool.h
  Representations/RepresentationUpdater.h
  Representations/RepresentationParallelUpdater.h
  Representations/RepresentationWindow.h
  Representations/Managers/TemporalManager.h
  Selectors/Selector.h
  Selectors/PixelSelector.h
  Utils/ColorRange.h
  Utils/Timer.h
  View/EventHandler.h
  EventHandlers/PointTracker.h
  EventHandlers/Brush.h
  EventHandlers/MaskPainter.h
  EventHandlers/BrushPainter.h
  EventHandlers/StrokePainter.h
  EventHandlers/ContourPainter.h
  View/RenderView.h
  View/Selection.h
  View/View2D.h
  View/ViewState.h
  View/View3D.h
  View/Widgets/Measures/MeasureEventHandler.h
  View/Widgets/Measures/MeasureWidget.h
  View/Widgets/PlanarSplit/PlanarSplitEventHandler.h
  View/Widgets/OrthogonalRegion/OrthogonalWidget2D.h
  View/Widgets/OrthogonalRegion/OrthogonalRepresentation.h
  View/Widgets/OrthogonalRegion/OrthogonalSliceSelector.h
  View/Widgets/ROI/ROIWidget.h
  View/Widgets/Skeleton/SkeletonWidget2D.h
  View/Widgets/Skeleton/SkeletonEventHandler.h
  View/CoordinateSystem.h
  View/Widgets/SelectionMeasure/Widget2D.h
  View/Widgets/SelectionMeasure/Widget3D.h
  View/Widgets/Contour/ContourWidget2D.h
  View/Widgets/Zoom/ZoomWidget2D.h
  View/Widgets/Zoom/ZoomWidget3D.h  
  View/Widgets/Zoom/ZoomEventHandler.h  
  Widgets/CategorySelector.h
  Widgets/CheckableTableView.h
  Widgets/CheckableTreeView.h
  Widgets/ColorBar.h
  Widgets/DrawingWidget.h
  Widgets/DoubleSpinBoxAction.h
  Widgets/HistogramBarsView.h
  Widgets/HistogramTreeMapView.h
  Widgets/HueSelector.h
  Widgets/InformationSelector.h
  Widgets/LineEditWithClear.h
  Widgets/NoteEditor.h
  Widgets/ProgressAction.h
  Widgets/PixelValueSelector.h
  Widgets/QComboTreeView.h
  Widgets/TagSelector.h
  Widgets/SchedulerProgress.h
  Widgets/SpinBoxAction.h
  Widgets/NumericalInput.h
  Widgets/ToolButton.h
  Widgets/TaskProgress.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt5_wrap_ui (GUI_UI
  Dialogs/CategorySelectorDialog/CategorySelectorDialog.ui
  Dialogs/RangeDefinitionDialog/ColorEngineRangeDefinitionDialog.ui
  Dialogs/RangeLimitsDialog/ColorEngineRangeLimitsDialog.ui
  Dialogs/ImageResolutionDialog/ImageResolutionDialog.ui
  Widgets/InformationSelector.ui
  Widgets/NoteEditor.ui
  Widgets/TagSelector.ui
  Widgets/SchedulerProgress.ui
  Widgets/TaskProgress.ui
  )

set (GUI_SOURCES
  ${GUI_MOC}
  ${GUI_UI}
  ColorEngines/CategoryColorEngine.cpp
  ColorEngines/ConnectionsColorEngine.cpp
  ColorEngines/IntensitySelectionHighlighter.cpp
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/InformationColorEngine.cpp
  ColorEngines/SelectionHighlighter.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/UserColorEngine.cpp
  Dialogs/DefaultDialogs.cpp
  Dialogs/CategorySelectorDialog/CategorySelectorDialog.cpp
  Dialogs/RangeDefinitionDialog/ColorEngineRangeDefinitionDialog.cpp
  Dialogs/RangeLimitsDialog/ColorEngineRangeLimitsDialog.cpp
  Dialogs/ImageResolutionDialog/ImageResolutionDialog.cpp
  Extension/Visualization/VisualizationState.cpp
  Model/CategoryAdapter.cpp
  Model/ChannelAdapter.cpp
  Model/ClassificationAdapter.cpp
  Model/ModelAdapter.cpp
  Model/Proxies/ChannelProxy.cpp
  Model/Proxies/ClassificationProxy.cpp
  Model/Proxies/InformationProxy.cpp
  Model/Proxies/LocationProxy.cpp
  Model/SampleAdapter.cpp
  Model/SegmentationAdapter.cpp
  Model/Utils/DBVH.cpp
  Model/Utils/ModelUtils.cpp
  Model/Utils/SegmentationLocator.cpp
  Model/Utils/SegmentationUtils.cpp
  Model/Utils/QueryAdapter.cpp
  Model/ViewItemAdapter.cpp
  Model/NeuroItemAdapter.cpp
  ModelFactory.cpp
  Representations/Pipelines/ChannelSlicePipeline.cpp
  Representations/Pipelines/SegmentationSlicePipeline.cpp
  Representations/Pipelines/SegmentationMeshPipeline.cpp
  Representations/Pipelines/SegmentationSmoothedMeshPipeline.cpp
  Representations/Pipelines/SegmentationContourPipeline.cpp
  Representations/Pipelines/SegmentationSkeletonPipelineBase.cpp
  Representations/Pipelines/SegmentationSkeleton2DPipeline.cpp
  Representations/Pipelines/SegmentationSkeleton3DPipeline.cpp
  Representations/Pipelines/SegmentationVolumetricCPUPipeline.cpp
  Representations/Pipelines/SegmentationVolumetricGPUPipeline.cpp
  Representations/Settings/PipelineStateUtils.cpp
  Representations/Settings/SegmentationSlicePoolSettings.cpp
  Representations/Settings/SegmentationContourPoolSettings.cpp
  Representations/Settings/SegmentationMeshPoolSettings.cpp
  Representations/Settings/SegmentationSkeletonPoolSettings.cpp
  Representations/Settings/ConnectionPoolSettings.cpp
  Representations/Pools/BufferedRepresentationPool.cpp
  Representations/Managers/ConnectionsManager.cpp
  Representations/Managers/CrosshairManager.cpp 
  Representations/Managers/TemporalManager.cpp
  Representations/ModelSources.cpp
  Representations/PipelineSources.cpp
  Representations/ManualPipelineSources.cpp
  Representations/RepresentationManager.cpp
  Representations/Managers/PoolManager.cpp
  Representations/Managers/PassiveActorManager.cpp
  Representations/RepresentationPool.cpp
  Representations/RepresentationState.cpp
  Representations/RepresentationUpdater.cpp
  Representations/RepresentationParallelUpdater.cpp
  Representations/RepresentationWindow.cpp
  Representations/Frame.cpp
  Selectors/PixelSelector.cpp
  Selectors/Selector.cpp
  Utils/ColorRange.cpp
  Utils/DefaultIcons.cpp
  Utils/QtModelUtils.cpp
  Utils/RepresentationUtils.cpp
  Utils/Timer.cpp
  Utils/EventUtils.cpp
  Utils/MiscUtils.cpp
  EventHandlers/Brush.cpp
  EventHandlers/CircularBrush.cpp
  EventHandlers/SphericalBrush.cpp
  EventHandlers/MaskPainter.cpp
  EventHandlers/PointTracker.cpp
  EventHandlers/BrushPainter.cpp
  EventHandlers/StrokePainter.cpp
  EventHandlers/ContourPainter.cpp
  View/EventHandler.cpp
  View/RenderView.cpp
  View/Selection.cpp
  View/View2D.cpp
  View/PlanarBehaviour.cpp
  View/ViewState.cpp
  View/Utils.cpp
  View/View3D.cpp
  View/Widgets/EspinaWidget.cpp
  View/Widgets/Contour/ContourWidget2D.cpp
  View/Widgets/Contour/vtkPlaneContourRepresentation.cpp
  View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.cpp
  View/Widgets/Contour/vtkPlaneContourWidget.cpp
  View/Widgets/Contour/vtkContourToPolygonFilter.cxx
  View/Widgets/Zoom/ZoomWidget2D.cpp
  View/Widgets/Zoom/ZoomWidget3D.cpp
  View/Widgets/Zoom/ZoomEventHandler.cpp
  View/Widgets/Zoom/vtkZoomSelectionWidget.cpp
  View/Widgets/Zoom/vtkZoomSelectionWidgetRepresentation.cpp
  View/Widgets/Measures/MeasureWidget.cpp
  View/Widgets/Measures/MeasureEventHandler.cpp
  View/Widgets/PlanarSplit/PlanarSplitEventHandler.cpp
  View/Widgets/PlanarSplit/PlanarSplitWidget.cpp
  View/Widgets/PlanarSplit/PlanarSplitWidget2D.cpp
  View/Widgets/PlanarSplit/PlanarSplitWidget3D.cpp
  View/Widgets/PlanarSplit/vtkPlanarSplitRepresentation2D.cpp
  View/Widgets/PlanarSplit/vtkPlanarSplitWidget.cpp
  View/Widgets/OrthogonalRegion/OrthogonalRepresentation.cpp
  View/Widgets/OrthogonalRegion/OrthogonalWidget2D.cpp
  View/Widgets/OrthogonalRegion/OrthogonalSliceSelector.cpp
  View/Widgets/OrthogonalRegion/vtkOrthogonalWidget2D.cpp
  View/Widgets/OrthogonalRegion/vtkOrthogonalRepresentation2D.cpp
  View/Widgets/ROI/ROIWidget.cpp
  View/Widgets/SelectionMeasure/Widget2D.cpp
  View/Widgets/SelectionMeasure/Widget3D.cpp
  View/Widgets/SelectionMeasure/vtkWidget2D.cpp
  View/Widgets/SelectionMeasure/vtkWidget3D.cpp
  View/Widgets/Skeleton/SkeletonWidget2D.cpp
  View/Widgets/Skeleton/SkeletonEventHandler.cpp
  View/Widgets/Skeleton/vtkSkeletonWidget.cpp
  View/Widgets/Skeleton/vtkSkeletonWidgetRepresentation.cpp
  View/vtkInteractorStyleEspinaSlice.cpp
  Widgets/CategorySelector.cpp
  Widgets/CheckableTableView.cpp
  Widgets/CheckableTreeView.cpp
  Widgets/ColorBar.cpp
  Widgets/DrawingWidget.cpp
  Widgets/DoubleSpinBoxAction.cpp
  Widgets/HistogramBarsView.cpp
  Widgets/HistogramTreeMapView.cpp
  Widgets/HueSelector.cpp
  Widgets/InformationSelector.cpp
  Widgets/LineEditWithClear.cpp
  Widgets/NoteEditor.cpp 
  Widgets/ProgressAction.cpp
  Widgets/PixelValueSelector.cpp
  Widgets/QComboTreeView.cpp
  Widgets/TagSelector.cpp 
  Widgets/SchedulerProgress.cpp
  Widgets/SpinBoxAction.cpp
  Widgets/Styles.cpp
  Widgets/ToolButton.cpp
  Widgets/NumericalInput.cpp
  Widgets/TaskProgress.cpp
  View/CoordinateSystem.cpp
  )

set(GUI_EXTERNAL_LIBS
  EspinaCore
  EspinaExtensions
  EspinaFilters
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(GUI_SOURCES ${GUI_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW)

if(APPLE)
   include_directories ( /System/Library/Frameworks )
   find_library(COCOA_LIBRARY Cocoa)
   find_library(GLUT_LIBRARY GLUT )
   find_library(OpenGL_LIBRARY OpenGL )
   mark_as_advanced (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   set(GUI_EXTERNAL_LIBS ${GUI_EXTERNAL_LIBS} ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
endif (APPLE)


add_library (EspinaGUI SHARED ${GUI_SOURCES})

target_link_libraries (EspinaGUI ${GUI_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
  EspinaGUI
  BASE_NAME         EspinaGUI
  EXPORT_MACRO_NAME EspinaGUI_EXPORT
  EXPORT_FILE_NAME  EspinaGUI_Export.h
  )

install(TARGETS EspinaGUI
  # IMPORTANT: Add the EspinaGUI library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")
