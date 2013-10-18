project (EspINA)

cmake_minimum_required (VERSION 2.8.6)

add_definitions(
  -std=c++11
  -fpermissive
  )

# needed for plugin compilation
set (Espina_DIR ${PROJECT_BINARY_DIR})

# Version Number
set (ESPINA_VERSION_MAJOR 1)
set (ESPINA_VERSION_MINOR 3)
set (ESPINA_VERSION_PATCH 2)


set (TEST_ESPINA_MODELS CACHE BOOL "Build EspINA Model Test wrappers")

# Configure Doxygen API Documentation
set (BUILD_DOCUMENTATION CACHE BOOL "Build EspINA API Documentation")

if (BUILD_DOCUMENTATION)
  find_package(Doxygen)

  if (DOXYGEN_FOUND)
    configure_file(
      "Doc/Doxyfile.in"
      "${PROJECT_BINARY_DIR}/Doxyfile"
      )
    set(DOXYGEN_INPUT ${PROJECT_BINARY_DIR}/Doxyfile)
    set(DOXYGEN_OUTPUT ${PROJECT_BINARY_DIR}/Doc)

    add_custom_command(
      OUTPUT ${DOXYGEN_OUTPUT}
      COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
      COMMAND DOXYGEN_OUTPUT_DIR=${DOXYGEN_OUTPUT} ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
      COMMAND ${CMAKE_COMMAND} -E echo "Done."
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      #DEPENDS ${DOXYGEN_INPUT} espina
      )

    add_custom_target(apidoc ALL DEPENDS ${DOXYGEN_OUTPUT})

  endif (DOXYGEN_FOUND)
endif (BUILD_DOCUMENTATION)

# Notify basic plugins to avoid searching for EspINA CMake files
set (COMPILING_ESPINA 1)

# Libraries and plugins output directory
set (ESPINA_LIBRARY_DIRECTORIES ${PROJECT_BINARY_DIR}/libs )

include_directories (${CMAKE_SOURCE_DIR})
include_directories (${PROJECT_BINARY_DIR}/Core)
include_directories (${PROJECT_BINARY_DIR}/GUI)
include_directories (${PROJECT_BINARY_DIR}/Undo)
link_directories    (${ESPINA_LIBRARY_DIRECTORIES})

# Build EspINA plugins in the correct directory
cmake_policy(SET CMP0013 OLD)
if (MINGW OR MSVC)
  set (LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/App"             )
else (MINGW OR MSVC)
  set (LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/libs"             )
endif (MINGW OR MSVC)

set (ESPINA_PLUGINS_DIR    "${PROJECT_BINARY_DIR}/App/plugins"    )
set (ESPINA_INCLUDE        "${PROJECT_SOURCE_DIR}")
set (ESPINA_BINARY_INCLUDE "${PROJECT_BINARY_DIR}")
set (ESPINA_USE_FILE       "${PROJECT_SOURCE_DIR}/EspinaUse.cmake")

configure_file(
  "${PROJECT_SOURCE_DIR}/EspinaConfig.h.in"
  "${PROJECT_BINARY_DIR}/EspinaConfig.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/EspinaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/EspinaConfig.cmake"
  )


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

find_package(Quazip REQUIRED)
# WIN64: deshabilitado, configurar a mano. Provoca el error "include called with wrong number of arguments.  Include only takes one file"
if (NOT DEFINED MINGW AND NOT DEFINED MSVC)
  include(${QUAZIP_USE_FILE})
endif (NOT DEFINED MINGW AND NOT DEFINED MSVC)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  graph regex)
link_directories ( ${Boost_LIBRARY_DIRS} )

#NOTE: We can't use ITK and BOOST graph at the same time because of
#      ITK_REQUIRED_CXX_FLAG: -ftemplate-depth-50
#      so we override CXX_FLAGS to unset previous flag
#set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -msse2")
if (NOT DEFINED MINGW AND NOT DEFINED MSVC)
  set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated")
else(NOT DEFINED MINGW AND NOT DEFINED MSVC)
  if (DEFINED MINGW)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/inno-setup/EspINA-Mingw64.iss.in" "${CMAKE_CURRENT_BINARY_DIR}/tools/inno-setup/EspINA-Mingw64.iss")
  endif (DEFINED MINGW)
  if (DEFINED MSVC)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/inno-setup/EspINA-MSVC.iss.in" "${CMAKE_CURRENT_BINARY_DIR}/tools/inno-setup/EspINA-MSVC.iss")
  endif(DEFINED MSVC)
endif (NOT DEFINED MINGW AND NOT DEFINED MSVC)



if (UNIX AND NOT DEFINED APPLE AND NOT DEFINED MSVC) # Check if it works on OS X, it has only been tested in Unix
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
  message(STATUS "CXX_FLAGS" ${CMAKE_CXX_FLAGS})
endif()


add_subdirectory( Core    )
#add_subdirectory( GUI     )
#add_subdirectory( Undo    )
#add_subdirectory( App     )
#add_subdirectory( Plugins )

# Testing
set (BUILD_TESTING OFF  CACHE BOOL "Build EspINA Unitary Tests")

if (BUILD_TESTING)

  include (CTest)

  configure_file(
    "${PROJECT_SOURCE_DIR}/CTestCustom.cmake"
    "${PROJECT_BINARY_DIR}/CTestCustom.cmake"
    )
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
