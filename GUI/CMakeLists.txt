include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
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
  QtWidget/SegmentationContextualMenu.h
  QtWidget/SliceView.h
  QtWidget/VolumeView.h
  Representations/SliceRepresentation.h
  Renderers/Renderer.h
  Renderers/MeshRenderer.h
  Renderers/SmoothedMeshRenderer.h
  vtkWidgets/RectangularRegion.h
  vtkWidgets/RectangularRegionSliceSelector.h
  ViewManager.h
  vtkWidgets/TubularWidget.h
  vtkWidgets/ContourWidget.h
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
  Pickers/BrushPicker.cpp
  Pickers/ContourSelector.cpp
  Pickers/ISelector.cpp
  Pickers/PixelSelector.cpp
  QtWidget/ActionSelector.cpp
  QtWidget/ActionSelectorWidget.cpp
  QtWidget/CheckableTableView.cpp
  QtWidget/CheckableTreeView.cpp
  QtWidget/EspinaRenderView.cpp
  QtWidget/HueSelector.cpp
  QtWidget/NoteEditor.cpp 
  QtWidget/TagSelector.cpp 
  QtWidget/QComboTreeView.cpp
  QtWidget/SegmentationContextualMenu.cpp
  QtWidget/SliceView.cpp
  QtWidget/SliceViewState.cpp
  QtWidget/VolumeView.cpp
  QtWidget/vtkInteractorStyleEspinaSlice.cpp
  Representations/IEspinaRepresentation.cpp
  Representations/SliceRepresentation.cpp 
  Renderers/CrosshairRenderer.cpp
  Renderers/MeshRenderer.cpp
  Renderers/SmoothedMeshRenderer.cpp
  Renderers/VolumetricRenderer.cpp
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
  )

set(GUI_EXTERNAL_LIBS
  EspinaCore
  EspinaFilters
)

add_library           (EspinaGUI SHARED ${GUI_SOURCES})
target_link_libraries (EspinaGUI ${GUI_EXTERNAL_LIBS})
