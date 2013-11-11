# Slice View Tests
create_test_sourcelist(TEST_SOURCES View3D_Tests.cpp # this file is created by this command
  view3D_default_constructor.cpp
)

include_directories(
#   ${SUBJECT_DIR}
 ${CMAKE_BINARY_DIR}/GUI
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${GUI_DIR}/ColorEngines/ColorEngine.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/OutputAdapter.h
  ${GUI_DIR}/Model/SegmentationAdapter.h
  ${GUI_DIR}/View/RenderView.h
  ${GUI_DIR}/View/View3D.h
)

set(LIB_DEPENDECIES
  EspinaCore
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
#   ${LIB_DEPENDECIES}
)

add_executable(View3D_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${GUI_DIR}/ColorEngines/NumberColorEngine.cpp
  ${GUI_DIR}/Extension/Visualization/VisualizationState.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/OutputAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/Representations/Renderers/Renderer.cpp
  ${GUI_DIR}/Representations/Representation.cpp
  ${GUI_DIR}/Selectors/Selector.cpp
  ${GUI_DIR}/View/RenderView.cpp
  ${GUI_DIR}/View/View3D.cpp
  ${GUI_DIR}/View/Widgets/EspinaWidget.cpp
  ${GUI_DIR}/View/vtkInteractorStyleEspinaSlice.cpp
  )

target_link_libraries(View3D_Tests ${LIB_DEPENDECIES} )

add_test("\"View 3D: Default Constructor\"" View3D_Tests view3D_default_constructor)