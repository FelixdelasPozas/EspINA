project(SEGMHA_IMPORTER_PLUGIN)

include (GenerateExportHeader)

# Version Number
set (PLUGIN_VERSION_MAJOR 1)
set (PLUGIN_VERSION_MINOR 0)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

IF (NOT COMPILING_ESPINA)
  find_package(Espina REQUIRED)
  include(${ESPINA_USE_FILE})
ENDIF (NOT COMPILING_ESPINA)

include_directories (
  ${CMAKE_CURRENT_BINARY_DIR} # Export file
  ${CMAKE_CURRENT_BINARY_DIR}/../CountingFrame # Export file
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

qt4_wrap_cpp(MOC_SOURCES
  SegmhaImporterPlugin.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
)

set (PLUGIN_SOURCES
  ${MOC_SOURCES}
  SegmhaImporterPlugin.cpp
  SegmhaReader.cpp
  )

set (PLUGIN_LIBRARIES
  EspinaCore
  EspinaGUI
  EspinaUndo
  EspinaSupport
  CountingFramePlugin
  )

if(DEFINED MINGW OR DEFINED MSVC)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW OR DEFINED MSVC)

add_library(SegmhaImporterPlugin SHARED ${PLUGIN_SOURCES})
GENERATE_EXPORT_HEADER(
             SegmhaImporterPlugin
             BASE_NAME SegmhaImporterPlugin
             EXPORT_MACRO_NAME SegmhaImporterPlugin_EXPORT
             EXPORT_FILE_NAME SegmhaImporterPlugin_Export.h
  )
target_link_libraries(SegmhaImporterPlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(SegmhaImporterPlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

install (
  TARGETS SegmhaImporterPlugin
  DESTINATION ${INSTALL_BIN_DIR}/plugins
  )
#
# set (CPACK_PACKAGE_VERSION_MAJOR ${PLUGIN_VERSION_MAJOR})
# set (CPACK_PACKAGE_VERSION_MINOR ${PLUGIN_VERSION_MINOR})
# set (CPACK_PACKAGE_VERSION_PATCH ${PLUGIN_VERSION_PATCH})
# set (CPACK_PACKAGE_NAME "ESPINA-SegmhaImporter")
# set (CPACK_GENERATOR "DEB")
# set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
# set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Import segmha segmentation files generated with python's ESPINA")
#
# include (CPack)
