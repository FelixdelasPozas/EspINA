project (EspINA)

cmake_minimum_required (VERSION 2.8)

# needed for plugin compilation
set (Espina_DIR ${PROJECT_BINARY_DIR})

# Version Number
set (ESPINA_VERSION_MAJOR 1)
set (ESPINA_VERSION_MINOR 2)
set (ESPINA_VERSION_PATCH 8)


set (TEST_ESPINA_MODELS CACHE BOOL "Build EspINA Model Test wrappers")
set (BUILD_TESTING OFF  CACHE BOOL "Build EspINA Unitary Tests")
set (USE_DASHBOARD OFF  CACHE BOOL "Use EspINA Unitary Tests Dashboard")

set (COMPILING_ESPINA 1)

set (ESPINA_LIBRARY_DIRECTORIES ${PROJECT_BINARY_DIR}/libs )

include_directories (${CMAKE_SOURCE_DIR})
link_directories    (${ESPINA_LIBRARY_DIRECTORIES})


# for building espina plugins in the correct directory
cmake_policy(SET CMP0013 OLD)

set (LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/libs"             )
set (ESPINA_PLUGINS_DIR  "${PROJECT_BINARY_DIR}/App/plugins"    )
set (ESPINA_INCLUDE      "${PROJECT_SOURCE_DIR}"                )
set (ESPINA_USE_FILE     "${PROJECT_SOURCE_DIR}/EspinaUse.cmake")

configure_file(
  "${PROJECT_SOURCE_DIR}/EspinaConfig.h.in"
  "${PROJECT_BINARY_DIR}/EspinaConfig.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/EspinaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/EspinaConfig.cmake"
  )

if (${USE_DASHBOARD})

  set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()

find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  graph)
link_directories ( ${Boost_LIBRARY_DIRS} )

#NOTE: We can't use ITK and BOOST graph at the same time because of
#      ITK_REQUIRED_CXX_FLAG: -ftemplate-depth-50
#      so we override CXX_FLAGS to unset previous flag
#set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -msse2")
set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated")


add_subdirectory( Core    )
add_subdirectory( Filters )
add_subdirectory( GUI     )
add_subdirectory( Undo    )
add_subdirectory( App     )
add_subdirectory( Plugins )


if (BUILD_TESTING)

 include (CTest)
 enable_testing()

 add_subdirectory(Tests)

endif()

### TODO: Review from previous version
# TRANSLATIONS
# set( DEFAULT_TRANSLATION_DIR "/home/lokifacio/tmp" )
# # Generate translation file
#  add_custom_command( OUTPUT es_ES.qm #TARGET DEFAULT_TRANSLATION_DIR POST_BUILD
#        COMMAND lrelease ${PROJECT_SOURCE_DIR}/frontend/translationFiles/translation_es.ts -qm ${CMAKE_BINARY_DIR}/es_ES.qm
#  )
#  add_custom_target(es DEPENDS es_ES.qm)
#  add_dependencies(espina es)


#LINUX INSTALATION
#if (UNIX)
#  file (GLOB PARAVIEWFILES ${ParaView_BINARY_DIR}/bin/*.so*)
#  file (GLOB ITKFILES ${ITK_DIR}/bin/*.so*)
#  file (GLOB QUAZIPFILES ${Quazip_DIR}/*.so*)
#
#  install (
#    FILES ${PARAVIEWFILES}
#    DESTINATION lib/espina
#    COMPONENT espina
#    )
#  install (
#    FILES ${ITKFILES}
#    DESTINATION lib/espina
#    COMPONENT espina
#    )
#
#  install (
#    FILES ${QUAZIPFILES}
#    DESTINATION lib/espina
#    COMPONENT espina
#    )
#
#  install(
#    FILES espina.conf
#    DESTINATION ../etc/ld.so.conf.d/
#    COMPONENT espina
#    )
#
#  #install (FILES ${PROJECT_SOURCE_DIR}/frontend/translationFiles/es.qm DESTINATION share/espina/)
#
#  set (CPACK_PACKAGE_VERSION_MAJOR ${ESPINA_VERSION_MAJOR})
#  set (CPACK_PACKAGE_VERSION_MINOR ${ESPINA_VERSION_MINOR})
#  set (CPACK_PACKAGE_VERSION_PATCH ${ESPINA_VERSION_PATCH})
#  set (CPACK_PACKAGE_NAME "EspINA")
#  set (CPACK_GENERATOR "DEB")
#  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
#  set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "EspINA: EspINA Interactive Neuron Analyzer")
#  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.46.1,
#        libboost-filesystem1.46.1,
#        libboost-graph1.46.1,
#        quazip,
#        itk-review,
#        vtk-paraview (>= 5.8),
#        paraview (>= 3.14),
#        libqt4-webkit,
#        libqt4-test,
#        libqt4-svg,
#        libqt4-help,
#        libqt4-xmlpatterns")
#        #libopenmpi1.3")
#  set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/CMake/debian/postinst;")
#
##  set (CPACK_COMPONENTS_ALL espina SeedGrowSegmentation)
##  set (CPACK_COMPONENT_ESPINA_DISPLAY_NAME "EspINA Interactive Neuron Analyser")
##  set (CPACK_COMPONENT_ESPINA_DESCRIPTION  "Interactive Analysis Tool for FEM images")
##
##  set(CPACK_DEB_COMPONENT_INSTALL ON)
#
#  include (CPack)
#endif (UNIX)
