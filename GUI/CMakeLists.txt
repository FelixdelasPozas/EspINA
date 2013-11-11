include (GenerateExportHeader)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

qt4_wrap_cpp(GUI_MOC
  ColorEngines/ColorEngine.h
  ColorEngines/CategoryColorEngine.h
  Model/ModelAdapter.h
  Model/ItemAdapter.h
  Model/OutputAdapter.h
  Model/Proxies/ChannelProxy.h
  Representations/Renderers/Renderer.h
  Representations/Renderers/ContourRenderer.h
  Representations/MeshRepresentationBase.h
  Representations/MeshRepresentation.h
  Representations/SmoothedMeshRepresentation.h
  Representations/RepresentationEmptySettings.h
  Representations/SliceRepresentationSettings.h
  Representations/ContourRepresentationSettings.h
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
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (GUI_UI
  Representations/RepresentationEmptySettings.ui
  Representations/SliceRepresentationSettings.ui
  Representations/ContourRepresentationSettings.ui
  Widgets/NoteEditor.ui
  Widgets/TagSelector.ui
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
  Model/Proxies/ChannelProxy.cpp
  Model/SampleAdapter.cpp
  Model/SegmentationAdapter.cpp
  Model/ViewItemAdapter.cpp
  Model/Utils/ModelAdapterUtils.cpp
  ModelFactory.cpp
  Representations/BasicRepresentationFactory.cpp
  Representations/Renderers/Renderer.cpp
  Representations/Renderers/ContourRenderer.cpp
  Representations/Renderers/MeshRenderer.cpp
  Representations/Renderers/SliceRenderer.cpp
  Representations/ContourRepresentationSettings.cpp
  Representations/Representation.cpp
  Representations/RepresentationEmptySettings.cpp
  Representations/RepresentationFactoryGroup.cpp 
  Representations/ContourRepresentation.cpp
  Representations/SmoothedMeshRepresentation.cpp
  Representations/MeshRepresentationBase.cpp
  Representations/MeshRepresentation.cpp 
  Representations/SliceRepresentation.cpp
  Representations/SliceRepresentationSettings.cpp 
  Selectors/Selector.cpp
  Utils/QtModelUtils.cpp
  View/RenderView.cpp
  View/View2D.cpp
  View/View2DState.cpp
  View/View3D.cpp
  View/vtkInteractorStyleEspinaSlice.cpp
  View/Widgets/EspinaWidget.cpp
  View/Widgets/Contour/vtkVoxelContour2D.cpp
  Widgets/ActionSelector.cpp
  Widgets/ActionSelectorWidget.cpp
  Widgets/CheckableTableView.cpp
  Widgets/CheckableTreeView.cpp
  Widgets/HueSelector.cpp
  Widgets/NoteEditor.cpp 
  Widgets/QComboTreeView.cpp
  Widgets/TagSelector.cpp 
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