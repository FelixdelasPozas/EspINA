include (GenerateExportHeader)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(GUI_MOC
  ColorEngines/CategoryColorEngine.h
  ColorEngines/ColorEngine.h
  Model/ItemAdapter.h
  Model/ModelAdapter.h
  Model/ViewItemAdapter.h
  Model/Proxies/ChannelProxy.h
  Model/Proxies/ClassificationProxy.h
  Model/Proxies/InformationProxy.h
  Model/ViewItemAdapter.h
  Representations/PipelineMultiplexer.h
  Representations/PipelineSources.h
  Representations/RepresentationManager.h
  Representations/RepresentationPool.h
  Representations/RepresentationUpdater.h
  Representations/RepresentationWindow.h
  Selectors/Selector.h
  Selectors/PixelSelector.h
  Selectors/BrushSelector.h
  Selectors/CircularBrushSelector.h
  Selectors/SphericalBrushSelector.h
  View/EventHandler.h
  View/RenderView.h
  View/Selection.h
  View/View2D.h
  View/ViewState.h
  View/ViewRendererMenu.h
  View/View3D.h
  View/Widgets/PlanarSplit/PlanarSplitWidget.h
  View/Widgets/OrthogonalRegion/OrthogonalRegion.h
  View/Widgets/OrthogonalRegion/OrthogonalRegionSliceSelector.h
  View/Widgets/ROI/ROIWidget.h
  View/Widgets/Skeleton/SkeletonWidget.h
  View/Widgets/Ruler/RulerWidget.h
  Widgets/ActionSelector.h
  Widgets/ActionSelectorWidget.h
  Widgets/CategorySelector.h
  Widgets/CheckableTableView.h
  Widgets/CheckableTreeView.h
  Widgets/DoubleSpinBoxAction.h
  Widgets/HueSelector.h
  Widgets/InformationSelector.h
  Widgets/NoteEditor.h
  Widgets/QComboTreeView.h
  Widgets/TagSelector.h
  Widgets/RenderersSelector.h
  Widgets/SchedulerProgress.h
  Widgets/SpinBoxAction.h
  Widgets/SliderAction.h
  Widgets/TaskProgress.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (GUI_UI
  Dialogs/CategorySelectorDialog.ui
  Representations/RepresentationEmptySettings.ui
  Representations/SliceRepresentationSettings.ui
  Representations/ContourRepresentationSettings.ui
  Widgets/InformationSelector.ui
  Widgets/NoteEditor.ui
  Widgets/RenderersSelector.ui
  Widgets/TagSelector.ui
  Widgets/SchedulerProgress.ui
  Widgets/TaskProgress.ui
  )

set (GUI_SOURCES
  ${GUI_MOC}
  ${GUI_UI}
  ColorEngines/CategoryColorEngine.cpp
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/UserColorEngine.cpp
  Dialogs/CategorySelectorDialog.cpp
  Dialogs/DefaultDialogs.cpp
  Extension/Visualization/VisualizationState.cpp
  Model/CategoryAdapter.cpp
  Model/ChannelAdapter.cpp
  Model/ClassificationAdapter.cpp
  Model/ModelAdapter.cpp
  Model/Proxies/ChannelProxy.cpp
  Model/Proxies/ClassificationProxy.cpp
  Model/Proxies/InformationProxy.cpp
  Model/SampleAdapter.cpp
  Model/SegmentationAdapter.cpp
  Model/Utils/ModelAdapterUtils.cpp
  Model/Utils/QueryAdapter.cpp
  Model/ViewItemAdapter.cpp
  Model/NeuroItemAdapter.cpp
  ModelFactory.cpp
  Representations/ChannelPipeline.cpp
  Representations/PipelineMultiplexer.cpp
  Representations/PipelineSources.cpp
  Representations/RepresentationManager.cpp
  Representations/RepresentationPipeline.cpp
  Representations/RepresentationPool.cpp
  Representations/RepresentationUpdater.cpp
  Representations/RepresentationWindow.cpp
  Representations/SegmentationPipeline.cpp
  Selectors/BrushSelector.cpp
  Selectors/PixelSelector.cpp
  Selectors/Selector.cpp
  Selectors/CircularBrushSelector.cpp
  Selectors/SphericalBrushSelector.cpp
  Utils/DefaultIcons.cpp
  Utils/QtModelUtils.cpp
  View/EventHandler.cpp
  View/RenderView.cpp
  View/Selection.cpp
  View/View2D.cpp
  View/ViewRendererMenu.cpp
  View/View2DState.cpp
  View/ViewState.cpp
  View/View3D.cpp
  View/Widgets/Contour/vtkVoxelContour2D.cpp
  View/Widgets/Zoom/ZoomSelectionWidget.cpp
  View/Widgets/Zoom/vtkZoomSelectionWidget.cpp
  View/Widgets/Zoom/vtkZoomSelectionWidgetRepresentation.cpp
  View/Widgets/Measures/MeasureWidget.cpp
  View/Widgets/PlanarSplit/PlanarSplitWidget.cpp
  View/Widgets/PlanarSplit/vtkPlanarSplitRepresentation2D.cpp
  View/Widgets/PlanarSplit/vtkPlanarSplitWidget.cpp
  View/Widgets/OrthogonalRegion/OrthogonalRegion.cpp
  View/Widgets/OrthogonalRegion/OrthogonalRegionSliceSelector.cpp
  View/Widgets/OrthogonalRegion/vtkOrthogonalRegionSliceWidget.cpp
  View/Widgets/OrthogonalRegion/vtkOrthogonalRegionSliceRepresentation.cpp
  View/Widgets/ROI/ROIWidget.cpp
  View/Widgets/Ruler/RulerWidget.cpp
  View/Widgets/Ruler/vtkRulerWidget.cpp
  View/Widgets/Ruler/vtkRulerWidget3D.cpp
  View/Widgets/Skeleton/SkeletonWidget.cpp
  View/Widgets/Skeleton/vtkSkeletonWidget.cpp
  View/Widgets/Skeleton/vtkSkeletonWidgetRepresentation.cpp
  View/vtkInteractorStyleEspinaSlice.cpp
  Widgets/ActionSelector.cpp
  Widgets/ActionSelectorWidget.cpp
  Widgets/CategorySelector.cpp
  Widgets/CheckableTableView.cpp
  Widgets/CheckableTreeView.cpp
  Widgets/DoubleSpinBoxAction.cpp
  Widgets/HueSelector.cpp
  Widgets/InformationSelector.cpp
  Widgets/NoteEditor.cpp 
  Widgets/QComboTreeView.cpp
  Widgets/RenderersSelector.cpp
  Widgets/TagSelector.cpp 
  Widgets/SchedulerProgress.cpp
  Widgets/SpinBoxAction.cpp
  Widgets/SliderAction.cpp
  Widgets/TaskProgress.cpp
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
