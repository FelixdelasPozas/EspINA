cmake_minimum_required(VERSION 2.6)


find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

# ITK Support
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories (
  ${ITK_VTK_GLUE}
  ${ITK_DIR}/../
  ${ITK_DIR}/../Code/Review/
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
)

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  Espina
  vtkInfovis
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
)

QT4_WRAP_CPP(MOC_SRCS 
  AppositionPlane.h
#   SegmhaImporterFilter.h
)

ADD_PARAVIEW_AUTO_START(IFACES IFACE_SRCS
  CLASS_NAME AppositionPlane
  STARTUP    onStartup
  SHUTDOWN   onShutdown
)
set (VTKFILTER_SRCS
  vtkAppositionPlaneFilter.cpp
  vtkAppositionPlaneFeatures.cpp
)

set (PLUGIN_SRCS 
  AppositionPlane.cpp
  AppositionPlaneExtension.cpp
)

ADD_PARAVIEW_PLUGIN(
  AppositionPlane "1.0"
  SERVER_MANAGER_SOURCES ${VTKFILTER_SRCS}
  SERVER_MANAGER_XML AppositionPlane.xml
#   SOURCES ${LOCAL_SOURCES}
#   GUI_RESOURCES RemoteFilel.qrc
#  GUI_RESOURCE_FILES AppositionPlaneGUI.xml
  GUI_INTERFACES ${IFACES}
  SOURCES ${PLUGIN_SRCS} ${MOC_SRCS} ${IFACE_SRCS}
)

target_link_libraries(AppositionPlane ${PARAVIEW_ADDITIONAL_LIBRARIES}) #Required to be used in server mode

install (TARGETS AppositionPlane LIBRARY DESTINATION bin/plugins)

add_subdirectory(tests)
