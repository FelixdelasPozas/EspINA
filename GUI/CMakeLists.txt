find_package(XLSLIB REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
  ${XLSLIB_INCLUDE_DIRS}
)

qt4_wrap_cpp(GUI_MOC
  Analysis/InformationSelector.h
  Pickers/BrushPicker.h
  Pickers/ISelector.h
  QtWidget/ActionSelector.h
  QtWidget/ActionSelectorWidget.h
  QtWidget/CheckableTableView.h
  QtWidget/CheckableTreeView.h
  QtWidget/EspinaRenderView.h
  QtWidget/HueSelector.h
  QtWidget/QComboTreeView.h
  QtWidget/NoteEditor.h
  QtWidget/TagSelector.h
  QtWidget/SliceView.h
  QtWidget/VolumeView.h
  Representations/ContourRepresentation.h
  Representations/CrosshairRepresentation.h
  Representations/IMeshRepresentation.h
  Representations/SliceRepresentation.h
  Representations/SimpleMeshRepresentation.h
  Representations/SmoothedMeshRepresentation.h
  Representations/VolumeGPURepresentation.h
  Representations/VolumeRaycastRepresentation.h
  Renderers/Renderer.h
  vtkWidgets/RectangularRegion.h
  vtkWidgets/RectangularRegionSliceSelector.h
  ViewManager.h
  vtkWidgets/TubularWidget.h
  vtkWidgets/ContourWidget.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (GUI_UI
  Analysis/InformationSelector.ui
  QtWidget/HueSelector.ui
  QtWidget/NoteEditor.ui
  QtWidget/TagSelector.ui
  )

set (GUI_SOURCES
  ${GUI_MOC}
  ${GUI_UI}
  Analysis/InformationSelector.cpp
  Analysis/xlsUtils.cpp
  IO/EspinaIO.cpp
  Pickers/BrushPicker.cpp
  Pickers/ContourSelector.cpp
  Pickers/ISelector.cpp
  Pickers/PixelSelector.cpp
  Representations/BasicGraphicalRepresentationFactory.cpp 
  QtWidget/ActionSelector.cpp
  QtWidget/ActionSelectorWidget.cpp
  QtWidget/CheckableTableView.cpp
  QtWidget/CheckableTreeView.cpp
  QtWidget/EspinaRenderView.cpp
  QtWidget/HueSelector.cpp
  QtWidget/NoteEditor.cpp 
  QtWidget/TagSelector.cpp 
  QtWidget/QComboTreeView.cpp
  QtWidget/SliceView.cpp
  QtWidget/SliceViewState.cpp
  QtWidget/VolumeView.cpp
  QtWidget/vtkInteractorStyleEspinaSlice.cpp
  Representations/ContourRepresentation.cpp
  Representations/CrosshairRepresentation.cpp
  Representations/GraphicalRepresentation.cpp
  Representations/IMeshRepresentation.cpp
  Representations/SliceRepresentation.cpp
  Representations/SimpleMeshRepresentation.cpp
  Representations/SmoothedMeshRepresentation.cpp
  Representations/VolumeGPURepresentation.cpp
  Representations/VolumeRaycastRepresentation.cpp
  Renderers/ContourRenderer.cpp 
  Renderers/CrosshairRenderer.cpp
  Renderers/SliceRenderer.cpp
  Renderers/MeshRenderer.cpp
  Renderers/SmoothedMeshRenderer.cpp
  Renderers/VolumetricRenderer.cpp
  Renderers/VolumetricGPURenderer.cpp
  vtkWidgets/ContourWidget.cpp
  vtkWidgets/EspinaWidget.cpp
  vtkWidgets/PlanarSplitSliceWidget.cpp
  vtkWidgets/PlanarSplitWidget.cpp
  vtkWidgets/RectangularRegion.cpp
  vtkWidgets/RectangularRegionSliceSelector.cpp
  vtkWidgets/SliceContourWidget.cpp
  vtkWidgets/vtkClipClosedSurface.cxx
  vtkWidgets/vtkContourToPolygonFilter.cxx
  vtkWidgets/vtkNonRotatingBoxRepresentation.cpp
  vtkWidgets/vtkNonRotatingBoxWidget.cpp
  vtkWidgets/vtkPlanarSplitRepresentation2D.cpp
  vtkWidgets/vtkPlanarSplitWidget.cpp
  vtkWidgets/vtkPlaneContourRepresentation.cpp
  vtkWidgets/vtkPlaneContourRepresentationGlyph.cpp
  vtkWidgets/vtkPlaneContourWidget.cpp
  vtkWidgets/vtkRectangularSliceRepresentation.cpp
  vtkWidgets/vtkRectangularSliceWidget.cpp
  vtkWidgets/vtkZoomSelectionWidget.cpp
  vtkWidgets/vtkZoomSelectionWidgetRepresentation.cpp
  vtkWidgets/ZoomSelectionSliceWidget.cpp
  vtkWidgets/ZoomSelectionWidget.cpp
  ViewManager.cpp
  vtkWidgets/MeasureWidget.cpp
  vtkWidgets/MeasureSliceWidget.cpp
  vtkWidgets/TubularWidget.cpp
  vtkWidgets/TubularSliceWidget.cpp
  vtkWidgets/vtkTubeSource.cxx
  vtkWidgets/vtkTubularRepresentation.cpp
  vtkWidgets/vtkTubularWidget.cpp
  vtkWidgets/vtkVoxelContour2D.cpp
  )

set(GUI_EXTERNAL_LIBS
  EspinaCore
#EspinaUndo
  QVTK
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${XLSLIB_LIBRARIES}
)

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

add_library           (EspinaGUI SHARED ${GUI_SOURCES})
target_link_libraries (EspinaGUI ${GUI_EXTERNAL_LIBS})
