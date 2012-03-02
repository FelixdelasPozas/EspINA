set (MOC_SOURCES
  ${MOC_SOURCES}
  plugins/EspinaWidgets/pqNonRotatingBoxWidget.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
#   ${WIDGET_SOURCES}
  plugins/EspinaWidgets/pqNonRotatingBoxWidget.cpp
  plugins/EspinaWidgets/RectangularSelection.cpp
  plugins/EspinaWidgets/vtkNonRotatingBoxWidget.cpp #Required by RectangularSelection
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  plugins/EspinaWidgets/vtkNonRotatingBoxRepresentation.cpp
  plugins/EspinaWidgets/vtkNonRotatingBoxWidget.cpp
  PARENT_SCOPE
  )

set (PVCORE_IFACES
  ${PVCORE_IFACES}
  ${WIDGET_IFACES}
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  plugins/EspinaWidgets/EspinaWidgets.xml
  PARENT_SCOPE
  )