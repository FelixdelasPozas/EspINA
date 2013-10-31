include (GenerateExportHeader)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(GUI_MOC
#   Analysis/InformationSelector.h
#   Pickers/BrushPicker.h
  Selectors/Selector.h
#   QtWidget/ActionSelector.h
#   QtWidget/ActionSelectorWidget.h
#   QtWidget/CheckableTableView.h
#   QtWidget/CheckableTreeView.h
#   QtWidget/EspinaRenderView.h
#   QtWidget/HueSelector.h
#   QtWidget/QComboTreeView.h
#   QtWidget/NoteEditor.h
#   QtWidget/TagSelector.h
#   QtWidget/SegmentationVisualizationSettingsDialog.h
#   QtWidget/SliceView.h
#   QtWidget/VolumeView.h
#   Representations/ContourRepresentation.h
#   Representations/CrosshairRepresentation.h
#   Representations/IMeshRepresentation.h
#   Representations/SliceRepresentation.h
#   Representations/SimpleMeshRepresentation.h
#   Representations/SmoothedMeshRepresentation.h
#   Representations/VolumeGPURepresentation.h
#   Representations/VolumeRaycastRepresentation.h
#   Renderers/Renderer.h
#   vtkWidgets/RectangularRegion.h
#   vtkWidgets/RectangularRegionSliceSelector.h
#   ViewManager.h
#   vtkWidgets/TubularWidget.h
#   vtkWidgets/ContourWidget.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (GUI_UI
#   Analysis/InformationSelector.ui
#   QtWidget/HueSelector.ui
#   QtWidget/NoteEditor.ui
#   QtWidget/SegmentationVisualizationSettingsDialog.ui
#   QtWidget/TagSelector.ui
  Representations/RepresentationEmptySettings.ui
#   Representations/ContourRepresentationSettings.ui
#   Representations/SliceRepresentationSettings.ui
  )

set (GUI_SOURCES 
  ${GUI_MOC}
  ${GUI_UI}
  Model/SampleAdapter.cpp
  Model/ChannelAdapter.cpp
  Model/SegmentationAdapter.cpp
  Model/ClassificationAdapter.cpp
  Model/CategoryAdapter.cpp
  Model/OutputAdapter.cpp
  Model/ModelAdapter.cpp
#   Model/Proxies/ChannelProxy.cpp
#   Model/Proxies/ClassificationProxy.cpp
#   Model/Proxies/ConnectomicProxy.cpp
#   Model/Proxies/InformationProxy.cpp
#   Model/Proxies/LocationProxy.cpp
#   Model/Proxies/RelationProxy.cpp
  Representations/Representation.cpp
#   Representations/ContourRepresentation.cpp
#   Representations/ContourRepresentationSettings.cpp
#   Representations/CrosshairRepresentation.cpp
  Representations/RepresentationEmptySettings.cpp 
#   Representations/MeshRepresentationBase.cpp
#   Representations/SliceRepresentation.cpp
#   Representations/SliceRepresentationSettings.cpp 
#   Representations/MeshRepresentation.cpp
#   Representations/SmoothedMeshRepresentation.cpp
#   Representations/VolumetricGPURepresentation.cpp
#   Representations/VolumetricRepresentation.cpp
#   Representations/Renderers/ContourRenderer.cpp 
#   Representations/Renderers/CrosshairRenderer.cpp
#   Representations/Renderers/SliceRenderer.cpp
#   Representations/Renderers/MeshRenderer.cpp
#   Representations/Renderers/SmoothedMeshRenderer.cpp
#   Representations/Renderers/VolumetricRenderer.cpp
#   Representations/Renderers/VolumetricGPURenderer.cpp
  Selectors/Selector.cpp
  View/RenderView.cpp

#   Analysis/InformationSelector.cpp
#   Analysis/xlsUtils.cpp
#   Extensions/Visualization/VisualizationState.cpp
#   IO/EspinaIO.cpp
#   Pickers/BrushPicker.cpp
#   Pickers/ContourSelector.cpp
#   Pickers/ISelector.cpp
#   Pickers/PixelSelector.cpp
#   Representations/BasicGraphicalRepresentationFactory.cpp 
#   QtWidget/ActionSelector.cpp
#   QtWidget/ActionSelectorWidget.cpp
#   QtWidget/CheckableTableView.cpp
#   QtWidget/CheckableTreeView.cpp
#   QtWidget/EspinaRenderView.cpp
#   QtWidget/HueSelector.cpp
#   QtWidget/NoteEditor.cpp 
#   QtWidget/TagSelector.cpp 
#   QtWidget/QComboTreeView.cpp
#   QtWidget/SegmentationVisualizationSettingsDialog.cpp
#   QtWidget/SliceView.cpp
#   QtWidget/SliceViewState.cpp
#   QtWidget/VolumeView.cpp
#   QtWidget/vtkInteractorStyleEspinaSlice.cpp
#   vtkWidgets/ContourWidget.cpp
#   vtkWidgets/EspinaWidget.cpp
#   vtkWidgets/PlanarSplitSliceWidget.cpp
#   vtkWidgets/PlanarSplitWidget.cpp
#   vtkWidgets/RectangularRegion.cpp
#   vtkWidgets/RectangularRegionSliceSelector.cpp
#   vtkWidgets/RulerSliceWidget.cpp
#   vtkWidgets/RulerWidget.cpp
#   vtkWidgets/SliceContourWidget.cpp
#   vtkWidgets/vtkClipClosedSurface.cxx
#   vtkWidgets/vtkContourToPolygonFilter.cxx
#   vtkWidgets/vtkNonRotatingBoxRepresentation.cpp
#   vtkWidgets/vtkNonRotatingBoxWidget.cpp
#   vtkWidgets/vtkPlanarSplitRepresentation2D.cpp
#   vtkWidgets/vtkPlanarSplitWidget.cpp
#   vtkWidgets/vtkPlaneContourRepresentation.cpp
#   vtkWidgets/vtkPlaneContourRepresentationGlyph.cpp
#   vtkWidgets/vtkPlaneContourWidget.cpp
#   vtkWidgets/vtkRectangularSliceRepresentation.cpp
#   vtkWidgets/vtkRectangularSliceWidget.cpp
#   vtkWidgets/vtkRulerWidget.cpp
#   vtkWidgets/vtkRulerWidget3D.cpp
#   vtkWidgets/vtkZoomSelectionWidget.cpp
#   vtkWidgets/vtkZoomSelectionWidgetRepresentation.cpp
#   vtkWidgets/ZoomSelectionSliceWidget.cpp
#   vtkWidgets/ZoomSelectionWidget.cpp
#   ViewManager.cpp
#   vtkWidgets/MeasureWidget.cpp
#   vtkWidgets/MeasureSliceWidget.cpp
#   vtkWidgets/TubularWidget.cpp
#   vtkWidgets/TubularSliceWidget.cpp
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