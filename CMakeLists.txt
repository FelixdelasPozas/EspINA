project (ESPINA)

cmake_minimum_required (VERSION 2.6)

# Version Number
set (ESPINA_VERSION_MAJOR 1) 
set (ESPINA_VERSION_MINOR 0)
set (ESPINA_VERSION_PATCH 1)


find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package(Cajal REQUIRED)
include(${CAJAL_USE_FILE})

include (CTest)
ENABLE_TESTING()
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

SET (ITK_VTK_GLUE ${ITK_DIR}/../Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src)

add_subdirectory (common)
add_subdirectory (plugins)
add_subdirectory (frontend)

set( DEFAULT_TAXONOMY_PATH "/usr/share/espina/default_taxonomy.xml" )
file(COPY "${PROJECT_SOURCE_DIR}/default_taxonomy.xml" DESTINATION "${CMAKE_BINARY_DIR}")
configure_file(
  "${PROJECT_SOURCE_DIR}/common/Config.h.in"
  "${PROJECT_SOURCE_DIR}/common/Config.h"
)
# TRANSLATIONS
#set( DEFAULT_TRANSLATION_DIR "/usr/share/espina/translations/" )
configure_file(
  "${PROJECT_SOURCE_DIR}/frontend/Config.h.in"
  "${PROJECT_SOURCE_DIR}/frontend/Config.h"
)
# Generate translation file
add_custom_command( OUTPUT es_ES.qm #TARGET DEFAULT_TRANSLATION_DIR POST_BUILD 
	COMMAND lrelease ${PROJECT_SOURCE_DIR}/frontend/translationFiles/translation_es.ts -qm ${CMAKE_BINARY_DIR}/es_ES.qm
)
add_custom_target(es DEPENDS es_ES.qm)
add_dependencies(Espina es)
  
file (GLOB PARAVIEWFILES ${ParaView_BINARY_DIR}/bin/*.so*)
file (GLOB ITKFILES ${ITK_DIR}/bin/*.so*)
file (GLOB QUAZIPFILES ${Quazip_DIR}/*.so*)

install (FILES ${PARAVIEWFILES} DESTINATION lib/espina)
install (FILES ${ITKFILES} DESTINATION lib/espina)
install (FILES ${QUAZIPFILES} DESTINATION lib/espina)
install (FILES espina.conf DESTINATION ../etc/ld.so.conf.d/)
install (FILES ${PROJECT_SOURCE_DIR}/default_taxonomy.xml DESTINATION share/espina/)
#install (FILES ${PROJECT_SOURCE_DIR}/frontend/translationFiles/es.qm DESTINATION share/espina/)

set (CPACK_PACKAGE_VERSION_MAJOR ${ESPINA_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${ESPINA_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${ESPINA_VERSION_PATCH})
set (CPACK_PACKAGE_NAME EspINA )
set (CPACK_GENERATOR "DEB")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
set (CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.42.0,
	libboost-filesystem1.42.0,
	libboost-graph1.42.0,
	libqt4-webkit,
	libqt4-svg,
	libqt4-help,
	libqt4-xmlpatterns,
	libopenmpi1.3")
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/CMake/debian/postinst;")

include (CPack)

