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

set( DEFAULT_TAXONOMY_PATH "/usr/share/espina/default_taxonomy.xml" )
file(COPY "${PROJECT_SOURCE_DIR}/default_taxonomy.xml" DESTINATION "${CMAKE_BINARY_DIR}")
configure_file(
  "${PROJECT_SOURCE_DIR}/common/Config.h.in"
  "${PROJECT_SOURCE_DIR}/common/Config.h"
)


file (GLOB LIBFILES ${ParaView_BINARY_DIR}/bin/*.so*)
MESSAGE (STATUS "Library Files:" ${LIBFILES})

install (FILES ${LIBFILES} DESTINATION lib/espina)
install (FILES espina.conf DESTINATION /etc/ld.so.conf.d/)
install (FILES ${PROJECT_SOURCE_DIR}/default_taxonomy.xml DESTINATION /usr/share/espina/)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.42.0,
	libboost-filesystem1.42.0,
	libboost-graph1.42.0,
	libqt4-help,
	libqt4-xmlpatterns,
	libopenmpi1.3,
	libzip1")

include (CPack)
