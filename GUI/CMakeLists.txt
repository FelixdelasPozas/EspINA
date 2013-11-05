include (GenerateExportHeader)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(GUI_MOC
  ColorEngines/ColorEngine.h
  Representations/Renderers/Renderer.h
  Selectors/Selector.h
  View/RenderView.h
  View/View2D.h
  View/View3D.h
  Widgets/ActionSelector.h
  Widgets/ActionSelectorWidget.h
  Widgets/CheckableTableView.h
  Widgets/CheckableTreeView.h
  Widgets/HueSelector.h
  Widgets/NoteEditor.h
  Widgets/QComboTreeView.h
  Widgets/TagSelector.h
#   View/Widgets/Contour/ContourWidget.h
#   View/Widgets/RectangularRegion/RectangularRegion.h
#   View/Widgets/RectangularRegion/RectangularRegionSliceSelector.h
#   Analysis/InformationSelector.h
#   Pickers/BrushPicker.h
#   QtWidget/SegmentationVisualizationSettingsDialog.h
#   Representations/ContourRepresentation.h
#   Representations/CrosshairRepresentation.h
#   Representations/IMeshRepresentation.h
#   Representations/SimpleMeshRepresentation.h
#   Representations/SliceRepresentation.h
#   Representations/SmoothedMeshRepresentation.h
#   Representations/VolumeGPURepresentation.h
#   Representations/VolumeRaycastRepresentation.h
  Representations/Renderers/Renderer.h
  Representations/Renderers/SliceRenderer.h
  Representations/SliceRepresentationSettings.h
  Representations/RepresentationEmptySettings.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (GUI_UI
  Representations/RepresentationEmptySettings.ui
  Widgets/NoteEditor.ui
  Widgets/TagSelector.ui
#   Analysis/InformationSelector.ui
#   QtWidget/HueSelector.ui
#   QtWidget/SegmentationVisualizationSettingsDialog.ui
#   Representations/ContourRepresentationSettings.ui
  Representations/SliceRepresentationSettings.ui
  )

set (GUI_SOURCES 
  ${GUI_MOC}
  ${GUI_UI}
  ColorEngines/CategoryColorEngine.cpp
  ColorEngines/MultiColorEngine.cpp
  ColorEngines/NumberColorEngine.cpp
  ColorEngines/TransparencySelectionHighlighter.cpp
  ColorEngines/UserColorEngine.cpp
  Extension/Visualization/VisualizationState.cpp
  Model/CategoryAdapter.cpp
  Model/ChannelAdapter.cpp
  Model/ClassificationAdapter.cpp
  Model/ModelAdapter.cpp
  Model/OutputAdapter.cpp
  Model/SampleAdapter.cpp
  Model/SegmentationAdapter.cpp
  Representations/Renderers/Renderer.cpp
  Representations/Renderers/SliceRenderer.cpp
  Representations/Representation.cpp
  Representations/RepresentationEmptySettings.cpp 
  Selectors/Selector.cpp
  View/RenderView.cpp
  View/View2D.cpp
  View/View2DState.cpp
  View/View3D.cpp
  View/vtkInteractorStyleEspinaSlice.cpp
  Widgets/ActionSelector.cpp
  Widgets/ActionSelectorWidget.cpp
  Widgets/CheckableTableView.cpp
  Widgets/CheckableTreeView.cpp
  Widgets/HueSelector.cpp
  Widgets/NoteEditor.cpp 
  Widgets/QComboTreeView.cpp
  Widgets/TagSelector.cpp 
#   View/Widgets/Contour/ContourWidget.cpp
#   View/Widgets/Contour/SliceContourWidget.cpp
#   View/Widgets/Contour/vtkContourToPolygonFilter.cxx
#   View/Widgets/Contour/vtkPlaneContourRepresentation.cpp
#   View/Widgets/Contour/vtkPlaneContourWidget.cpp
#   View/Widgets/EspinaWidget.cpp
#   View/Widgets/PlanarSplit/PlanarSplitSliceWidget.cpp
#   View/Widgets/PlanarSplit/PlanarSplitWidget.cpp
#   View/Widgets/PlanarSplit/vtkPlanarSplitRepresentation2D.cpp
#   View/Widgets/PlanarSplit/vtkPlanarSplitWidget.cpp
#   View/Widgets/RectangularRegion/RectangularRegion.cpp
#   View/Widgets/RectangularRegion/RectangularRegionSliceSelector.cpp
#   View/Widgets/RectangularRegion/vtkRectangularSliceRepresentation.cpp
#   View/Widgets/RectangularRegion/vtkRectangularSliceWidget.cpp
#   View/Widgets/Ruler/RulerSliceWidget.cpp
#   View/Widgets/Ruler/RulerWidget.cpp
#   View/Widgets/Ruler/vtkRulerWidget.cpp
#   View/Widgets/Ruler/vtkRulerWidget3D.cpp
#   View/Widgets/Zoom/ZoomSelectionSliceWidget.cpp
#   View/Widgets/Zoom/ZoomSelectionWidget.cpp
#   View/Widgets/Zoom/vtkZoomSelectionWidget.cpp
#   View/Widgets/Zoom/vtkZoomSelectionWidgetRepresentation.cpp
#   Analysis/InformationSelector.cpp
#   Analysis/xlsUtils.cpp
#   Extensions/Visualization/VisualizationState.cpp
#   IO/EspinaIO.cpp
#   Model/Proxies/ChannelProxy.cpp
#   Model/Proxies/ClassificationProxy.cpp
#   Model/Proxies/ConnectomicProxy.cpp
#   Model/Proxies/InformationProxy.cpp
#   Model/Proxies/LocationProxy.cpp
#   Model/Proxies/RelationProxy.cpp
#   Pickers/BrushPicker.cpp
#   Pickers/ContourSelector.cpp
#   Pickers/ISelector.cpp
#   Pickers/PixelSelector.cpp
#   QtWidget/SegmentationVisualizationSettingsDialog.cpp
#   Representations/BasicGraphicalRepresentationFactory.cpp 
#   Representations/ContourRepresentation.cpp
#   Representations/ContourRepresentationSettings.cpp
#   Representations/CrosshairRepresentation.cpp
#   Representations/MeshRepresentation.cpp
#   Representations/MeshRepresentationBase.cpp
#   Representations/Renderers/ContourRenderer.cpp 
#   Representations/Renderers/CrosshairRenderer.cpp
#   Representations/Renderers/MeshRenderer.cpp
#   Representations/Renderers/SliceRenderer.cpp
#   Representations/Renderers/SmoothedMeshRenderer.cpp
#   Representations/Renderers/VolumetricGPURenderer.cpp
#   Representations/Renderers/VolumetricRenderer.cpp
  Representations/SliceRepresentation.cpp
  Representations/SliceRepresentationSettings.cpp 
#   Representations/SmoothedMeshRepresentation.cpp
#   Representations/VolumetricGPURepresentation.cpp
#   Representations/VolumetricRepresentation.cpp
#   View/Widgets//vtkClipClosedSurface.cxx
#   View/Widgets//vtkNonRotatingBoxRepresentation.cpp
#   View/Widgets//vtkNonRotatingBoxWidget.cpp
#   View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.cpp
#   vtkWidgets/MeasureSliceWidget.cpp
#   vtkWidgets/MeasureWidget.cpp
#   vtkWidgets/TubularSliceWidget.cpp
#   vtkWidgets/TubularWidget.cpp
#   vtkWidgets/vtkTubeSource.cxx
#   vtkWidgets/vtkTubularRepresentation.cpp
#   vtkWidgets/vtkTubularWidget.cpp
#   vtkWidgets/vtkVoxelContour2D.cpp
  )

set(GUI_EXTERNAL_LIBS
  EspinaCore
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW OR DEFINED MSVC)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(GUI_SOURCES ${GUI_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW OR DEFINED MSVC)

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
GENERATE_EXPORT_HEADER(
             EspinaGUI
             BASE_NAME EspinaGUI
             EXPORT_MACRO_NAME EspinaGUI_EXPORT
             EXPORT_FILE_NAME EspinaGUI_Export.h
			 )
target_link_libraries (EspinaGUI ${GUI_EXTERNAL_LIBS})