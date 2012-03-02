project( SEGMHA_IMPORTER_PLUGIN )

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

# ITK Support
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set (ITK_VTK_GLUE ${ITK_DIR}/../Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src)

find_package(Espina REQUIRED)
include(${ESPINA_USE_FILE})

include_directories (
  ${ITK_VTK_GLUE}
  ${ITK_DIR}/../Code/Review/
  )

QT4_WRAP_CPP(MOC_SOURCES
  SegmhaImporter.h
)

ADD_PARAVIEW_AUTO_START(IFACES IFACE_SOURCES
  CLASS_NAME SegmhaImporter
  STARTUP    onStartup
  SHUTDOWN   onShutdown
)

set (LOCAL_SOURCES
  SegmhaImporter.cpp
  SegmhaImporterFilter.cpp
  )

set (SERVER_SOURCES
  vtkExtractBlockAsImage.cpp
  vtkSegmhaReader.cpp
  )

set (PLUGIN_SOURCES
  ${MOC_SRCS}
  ${LOCAL_SOURCES}
  )

set (PLUGIN_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  EspinaCore
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
)


ADD_PARAVIEW_PLUGIN(
  SegmhaImporter
  "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}"
  SERVER_MANAGER_XML SegmhaReader.xml
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  GUI_RESOURCE_FILES SegmhaReaderGUI.xml
  SOURCES  ${PLUGIN_SOURCES} ${IFACE_SOURCES}
  GUI_INTERFACES ${IFACES}
)

target_link_libraries(SegmhaImporter
  ${PLUGIN_LIBRARIES} #Required to be used in server mode
  )

install (
  TARGETS SegmhaImporter
  LIBRARY
  DESTINATION bin/plugins
  )

set (CPACK_PACKAGE_VERSION_MAJOR ${PLUGIN_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PLUGIN_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PLUGIN_VERSION_PATCH})
set (CPACK_PACKAGE_NAME "EspINA-egmhaImporter")
set (CPACK_GENERATOR "DEB")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Import segmha segmentation files generated with python's EspINA")

include (CPack)