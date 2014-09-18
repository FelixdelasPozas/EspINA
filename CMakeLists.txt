cmake_minimum_required (VERSION 2.8.6)

if(CMAKE_MAJOR_VERSION EQUAL 3 OR CMAKE_MAJOR_VERSION GREATER 3)
  cmake_policy(SET CMP0048 OLD) # Don't use project version
endif()

project (ESPINA)


add_definitions(
  -std=c++11
  -fpermissive
  )

find_package(QuaZip REQUIRED)
#if (NOT DEFINED MINGW AND NOT DEFINED MSVC)
#  include(${QUAZIP_USE_FILE})
#endif (NOT DEFINED MINGW AND NOT DEFINED MSVC)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  graph regex)
link_directories ( ${Boost_LIBRARY_DIRS} )

# Notify basic plugins to avoid searching for ESPINA CMake files
set (COMPILING_ESPINA 1)

set (AUTOMATIC_VERSION TRUE CACHE BOOL "Detect ESPINA version automatically from git tags")

if (AUTOMATIC_VERSION)
 
  execute_process(COMMAND git describe --tags COMMAND cut -d. -f1 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE ESPINA_VERSION_MAJOR OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git describe --tags COMMAND cut -d. -f2 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE ESPINA_VERSION_MINOR OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git describe --tags COMMAND cut -d. -f3 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE ESPINA_VERSION_PATCH OUTPUT_STRIP_TRAILING_WHITESPACE)

endif ()

if (NOT AUTOMATIC_VERSION
    OR  ESPINA_VERSION_MAJOR STREQUAL ""
    OR  ESPINA_VERSION_MINOR STREQUAL ""
    OR  ESPINA_VERSION_PATCH STREQUAL "")
  
  # Version Number
  set (ESPINA_VERSION_MAJOR ${FOUND_ESPINA_VERSION_MAJOR} CACHE INTEGER "ESPINA major version")
  set (ESPINA_VERSION_MINOR ${FOUND_ESPINA_VERSION_MINOR} CACHE INTEGER "ESPINA minor version")
  set (ESPINA_VERSION_PATCH ${FOUND_ESPINA_VERSION_PATCH} CACHE INTEGER "ESPINA patch version")

endif ()

set (ESPINA_VERSION
  ${ESPINA_VERSION_MAJOR}.${ESPINA_VERSION_MINOR}.${ESPINA_VERSION_PATCH}
  )
message(STATUS "ESPINA Version: ${ESPINA_VERSION}")

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR     lib     CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR     bin     CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/espina)
endif()

set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

set (BUILD_DOCUMENTATION CACHE BOOL "Build EspINA API Documentation")
set (BUILD_TESTING OFF   CACHE BOOL "Build EspINA Unitary Tests")
set (METADONA_SUPPORT    CACHE BOOL "Build ESPINA with Meta Data Oriented Neuron Access support")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")



# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

include_directories (
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_BINARY_DIR}" # Needed for units created during compilation
  )

# Build EspINA plugins in the correct directory
cmake_policy(SET CMP0013 OLD)
if (MINGW OR MSVC)
  set (LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/App"             )
else (MINGW OR MSVC)
  set (LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/libs"             )
endif (MINGW OR MSVC)

set (ESPINA_PLUGINS_DIR    "${PROJECT_BINARY_DIR}/App/plugins")
set (ESPINA_INCLUDE        "${PROJECT_SOURCE_DIR}")
set (ESPINA_BINARY_INCLUDE "${PROJECT_BINARY_DIR}")
set (ESPINA_USE_FILE       "${PROJECT_SOURCE_DIR}/EspinaUse.cmake")
set (ESPINA_LIBRARY_DIRECTORIES ${PROJECT_BINARY_DIR}/libs ) # Libraries and plugins output directory

link_directories    (${ESPINA_LIBRARY_DIRECTORIES})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

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

set (METADONA_FOUND 0)
if  (METADONA_SUPPORT)
  find_package (Metadona)
  if (METADONA_FOUND)
    message (STATUS "Use Metadona")
    message (STATUS ${METADONA_USE_FILE})
    include (${METADONA_USE_FILE})
  endif (METADONA_FOUND)
endif (METADONA_SUPPORT)

configure_file(
  EspinaConfig.h.in
  "${PROJECT_BINARY_DIR}/EspinaConfig.h"
  )


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


add_subdirectory( Core       )
add_subdirectory( Extensions )
add_subdirectory( Filters    )
add_subdirectory( GUI        )
add_subdirectory( Support    )
add_subdirectory( Undo       )
add_subdirectory( App        )
add_subdirectory( Plugins    )

if (BUILD_TESTING)
  include (CTest)

  configure_file(
    "${PROJECT_SOURCE_DIR}/CTestCustom.cmake"
    "${PROJECT_BINARY_DIR}/CTestCustom.cmake"
    )
    
  # Generate coverage information
  set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
  
  set( CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
  
  add_subdirectory(Tests)

endif()

# CMake files to use this project by others
# ===============================
 
# Add all targets to the build-tree export set
export(TARGETS 
  EspinaCore 
  EspinaGUI 
  EspinaExtensions 
  EspinaFilters
  EspinaSupport
  EspinaUndo
  espina
  FILE "${PROJECT_BINARY_DIR}/ESPINATargets.cmake")
 
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE ESPINA)
 
# Create the ESPINAConfig.cmake and ESPINAConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}/espina"
   "${INSTALL_INCLUDE_DIR}")
file(RELATIVE_PATH REL_LIBRARIES_DIR "${INSTALL_CMAKE_DIR}/espina"
   "${INSTALL_LIB_DIR}")
# ... for the build tree
set(CONF_CMAKE_DIR      "${PROJECT_SOURCE_DIR}")
set(CONF_LIBRARIES_DIRS "${LIBRARY_OUTPUT_PATH}")
set(CONF_INCLUDE_DIRS   "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file (
  ESPINAConfig.cmake.in
  "${PROJECT_BINARY_DIR}/ESPINAConfig.cmake"
  )
# ... for the install tree
set(CONF_CMAKE_DIR      "\${ESPINA_CMAKE_DIR}")
set(CONF_LIBRARIES_DIRS "\${ESPINA_CMAKE_DIR}/${REL_LIBRARIES_DIR}")
set(CONF_INCLUDE_DIRS   "\${ESPINA_CMAKE_DIR}/${REL_INCLUDE_DIR}/espina")
configure_file(
  ESPINAConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ESPINAConfig.cmake" @ONLY)
# ... for both
configure_file(
  ESPINAConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/ESPINAConfigVersion.cmake" @ONLY)
 
# Install the ESPINAConfig.cmake and ESPINAConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ESPINAConfig.cmake"
  "${PROJECT_SOURCE_DIR}/ESPINAUse.cmake"
  "${PROJECT_BINARY_DIR}/ESPINAConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
install(EXPORT ESPINATargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)

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
