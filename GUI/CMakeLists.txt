include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(GUI_MOC
  Pickers/BrushPicker.h
  Pickers/IPicker.h
  QtWidget/ActionSelector.h
  QtWidget/ActionSelectorWidget.h
  QtWidget/CheckableTreeView.h
  QtWidget/EspinaRenderView.h
  QtWidget/HueSelector.h
  QtWidget/QComboTreeView.h
  QtWidget/SegmentationContextualMenu.h
  QtWidget/SliceView.h
  QtWidget/VolumeView.h
  Renderers/Renderer.h
  vtkWidgets/RectangularRegion.h
  vtkWidgets/RectangularRegionSliceSelector.h
  ViewManager.h
  vtkWidgets/TubularWidget.h
  Renderers/MeshRenderer.h
  )

qt4_wrap_ui (GUI_UI
  QtWidget/HueSelector.ui
  )

set (GUI_SOURCES
  ${GUI_MOC}
  ${GUI_UI}
  Pickers/BrushPicker.cpp
  Pickers/ContourPicker.cpp
  Pickers/IPicker.cpp
  Pickers/PixelPicker.cpp
  QtWidget/ActionSelector.cpp
  QtWidget/ActionSelectorWidget.cpp
  QtWidget/CheckableTreeView.cpp
  QtWidget/EspinaRenderView.cpp
  QtWidget/HueSelector.cpp
  QtWidget/QComboTreeView.cpp
  QtWidget/SegmentationContextualMenu.cpp
  QtWidget/SliceView.cpp
  QtWidget/SliceViewState.cpp
  QtWidget/VolumeView.cpp
  QtWidget/vtkInteractorStyleEspinaSlice.cpp
  Renderers/CrosshairRenderer.cpp
  Renderers/MeshRenderer.cpp
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
)

add_library           (EspinaGUI SHARED ${GUI_SOURCES})
target_link_libraries (EspinaGUI ${GUI_EXTERNAL_LIBS})
