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

include_directories (
	${ITK_VTK_GLUE}
        ${ITK_DIR}/../Code/Review/
	)

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  Espina
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
)

QT4_WRAP_CPP(MOC_SRCS 
  SegmhaReader.h
#   SegmhaImporterFilter.h
)

ADD_PARAVIEW_AUTO_START(IFACES IFACE_SRCS
  CLASS_NAME SegmhaReader
  STARTUP    onStartup
  SHUTDOWN   onShutdown
)

ADD_PARAVIEW_PLUGIN(
  SegmhaReader "1.0"
  SERVER_MANAGER_SOURCES vtkSegmhaReader.cpp vtkExtractBlockAsImage.cpp
  SERVER_MANAGER_XML SegmhaReader.xml
#   SOURCES ${LOCAL_SOURCES}
#   GUI_RESOURCES RemoteFilel.qrc
  GUI_RESOURCE_FILES SegmhaReaderGUI.xml
  GUI_INTERFACES ${IFACES}
  SOURCES SegmhaReader.cpp SegmhaImporterFilter.cpp ${READER_SRCS} ${MOC_SRCS} ${IFACE_SRCS}
)

target_link_libraries(SegmhaReader ${PARAVIEW_ADDITIONAL_LIBRARIES}) #Required to be used in server mode

install (TARGETS SegmhaReader LIBRARY DESTINATION bin/plugins)
