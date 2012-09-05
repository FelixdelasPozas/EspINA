project(SEGMHA_IMPORTER_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 2)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(Espina REQUIRED)
include(${ESPINA_USE_FILE})

include_directories (
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

qt4_wrap_cpp(MOC_SOURCES
  SegmhaImporter.h
)

set (PLUGIN_SOURCES
  SegmhaImporter.cpp
  SegmhaImporterFilter.cpp
  ${MOC_SOURCES}
  )

set (PLUGIN_LIBRARIES
  EspinaCore
  )

add_library(SegmhaImporterPlugin SHARED ${PLUGIN_SOURCES})
target_link_libraries(SegmhaImporterPlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(SegmhaImporterPlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

# install (
#   TARGETS SegmhaImporter
#   LIBRARY
#   DESTINATION bin/plugins
#   )
#
# set (CPACK_PACKAGE_VERSION_MAJOR ${PLUGIN_VERSION_MAJOR})
# set (CPACK_PACKAGE_VERSION_MINOR ${PLUGIN_VERSION_MINOR})
# set (CPACK_PACKAGE_VERSION_PATCH ${PLUGIN_VERSION_PATCH})
# set (CPACK_PACKAGE_NAME "EspINA-SegmhaImporter")
# set (CPACK_GENERATOR "DEB")
# set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
# set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Import segmha segmentation files generated with python's EspINA")
#
# include (CPack)