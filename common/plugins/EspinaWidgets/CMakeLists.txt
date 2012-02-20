# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SOURCES
  pqNonRotatingBoxWidget.h
  RectangularSelection.h
  )
# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  pqNonRotatingBoxWidget.ui
  )

ADD_3DWIDGET(
  WIDGET_IFACES WIDGET_SOURCES
  CLASS_NAME pqNonRotatingBoxWidget
  WIDGET_TYPE NonRotatingBox
  )

set (LOCAL_SOURCES
  pqNonRotatingBoxWidget.cpp
  RectangularSelection.cpp
  )

set (PLUGIN_SOURCES
  ${LOCAL_SOURCES}
  ${MOC_SOURCES}
  ${UI_BUILT_SOURCES}
  ${WIDGET_SOURCES}
  )

set (VTK_SOURCES
  vtkNonRotatingBoxRepresentation.cpp
  vtkNonRotatingBoxWidget.cpp
  )

ADD_PARAVIEW_PLUGIN(
  EspinaWidgets
  "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}"
  SERVER_MANAGER_XML EspinaWidgets.xml
  SERVER_MANAGER_SOURCES ${VTK_SOURCES}
  SOURCES ${PLUGIN_SOURCES} 
  GUI_INTERFACES ${WIDGET_IFACES}
  )

target_link_libraries (EspinaWidgets)

install (TARGETS EspinaWidgets
         LIBRARY
         DESTINATION bin/plugins
         COMPONENT espinaApp)
