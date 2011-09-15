# project (ESPINA_SEGMHA_FILE_PLUGIN)

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

ADD_PARAVIEW_PLUGIN(
  SegmhaFileReader "1.0"
  SERVER_MANAGER_SOURCES vtkSegmhaFileReader.cpp #vtkFileContent.cpp
  SERVER_MANAGER_XML SegmhaFileReader.xml
#   SOURCES ${LOCAL_SOURCES}
#   GUI_RESOURCES RemoteFilel.qrc
  GUI_RESOURCE_FILES SegmhaFileReaderGUI.xml
)

target_link_libraries(SegmhaFileReader ${PARAVIEW_ADDITIONAL_LIBRARIES}) #Required to be used in server mode

install (TARGETS SegmhaFileReader LIBRARY DESTINATION bin/plugins)
