find_package( ParaView REQUIRED )
include( ${PARAVIEW_USE_FILE} )
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

qt4_wrap_cpp( MOC_SRCS
  pqSliceView.h
  )

add_paraview_view_module(
  IFACES
  IFACE_SRCS
  VIEW_TYPE pqSliceView
  VIEW_XML_GROUP views
  )

set (SERVER_SOURCES
  vtkPVSliceView.cpp
  vtkSMSliceViewProxy.cpp
  vtkSliceRepresentation.cpp
  )

set (PLUGIN_SOURCES
  pqSliceView.cpp
  )

add_paraview_plugin( EspinaViews "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_MANAGER_XML EspinaViews.xml
  GUI_INTERFACES ${IFACES}
  GUI_SOURCES ${IFACE_SRCS} ${PLUGIN_SOURCES} ${MOC_SRCS}
  )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(EspinaViews PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

install (
  TARGETS EspinaViews
  LIBRARY
  DESTINATION bin/plugins
  COMPONENT espina
  )