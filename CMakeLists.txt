project (ESPINA)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package(Cajal REQUIRED)
include(${CAJAL_USE_FILE})

add_subdirectory (common)
add_subdirectory (plugins)
add_subdirectory (frontend)

file(COPY "${PROJECT_SOURCE_DIR}/default_taxonomy.xml" DESTINATION "${CMAKE_BINARY_DIR}")
set( DEFAULT_TAXONOMY_PATH "${CMAKE_BINARY_DIR}/default_taxonomy.xml" )
configure_file(
  "${PROJECT_SOURCE_DIR}/common/Config.h.in"
  "${PROJECT_SOURCE_DIR}/common/Config.h"
)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required

include (CPack)
