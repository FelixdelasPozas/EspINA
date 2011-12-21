find_package( ParaView REQUIRED )
include( ${PARAVIEW_USE_FILE} )
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

qt4_wrap_cpp( MOC_SRCS
  EspinaView.h
  )

add_paraview_view_module(
  IFACES
  IFACE_SRCS
  VIEW_TYPE EspinaView
  VIEW_XML_GROUP views
  )

set (SERVER_SOURCES
  vtkPVEspinaView.cpp
  vtkSMEspinaViewProxy.cpp
  vtkEspinaView.cpp
  vtkEspinaSliceRepresentation.cpp
  )

set (PLUGIN_SOURCES
  EspinaView.cpp
  )

add_paraview_plugin( EspinaViews "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_MANAGER_XML EspinaViews.xml
  GUI_INTERFACES ${IFACES}
  GUI_SOURCES ${IFACE_SRCS} ${PLUGIN_SOURCES} ${MOC_SRCS}
  )

message( STATUS ${IFACE_SRCS} )