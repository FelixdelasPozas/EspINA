project(APPOSITION_PLANE_PLUGIN)

include (GenerateExportHeader)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 2)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

  set (BUILD_TESTING OFF  CACHE BOOL "Build EspINA Unitary Tests")


if (NOT COMPILING_ESPINA)

  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})

  find_package (VTK REQUIRED)
  include(${VTK_USE_FILE})

  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})

  find_package(Espina REQUIRED)
  include(${ESPINA_USE_FILE})

endif ()

  find_package(XLSLIB REQUIRED)

include_directories (
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # UI files
  ${XLSLIB_INCLUDE_DIRS}
)

qt4_add_resources (RCC_BUILT_SOURCES 
  GUI/rsc/AppositionSurface.qrc
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  ${ESPINA_INCLUDE}/App/Dialogs/TabularReport/TabularReportEntry.ui
  GUI/Settings/AppositionSurfaceSettings.ui
#  GUI/FilterInspector/AppositionSurfaceFilterInspector.ui
  )

QT4_WRAP_CPP(MOC_SOURCES
  AppositionSurfacePlugin.h
  Filter/AppositionSurfaceFilter.h
  GUI/Settings/AppositionSurfaceSettings.h
  GUI/AppositionSurfaceToolGroup.h
#  GUI/FilterInspector/AppositionSurfaceFilterInspector.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
)

set (PLUGIN_SOURCES
  ${MOC_SOURCES}
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  AppositionSurfacePlugin.cpp
  Core/Extensions/AppositionSurfaceExtension.cpp
  Core/Extensions/ExtensionFactory.cpp
  Filter/AppositionSurfaceFilter.cpp
  GUI/Analysis/SASAnalysisDialog.cpp
  GUI/Analysis/SASTabularReport.cpp
  GUI/Analysis/SASInformationProxy.cpp
  GUI/AppositionSurfaceToolGroup.cpp
  GUI/Settings/AppositionSurfaceSettings.cpp
#  GUI/FilterInspector/AppositionSurfaceFilterInspector.cpp
)

set (PLUGIN_LIBRARIES
  EspinaCore
  EspinaGUI
  EspinaUndo
  EspinaSupport
  vtkInfovisCore
  ${XLSLIB_LIBRARIES}
  )

if(MINGW)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(MINGW)

add_library(AppositionSurfacePlugin SHARED ${PLUGIN_SOURCES})
GENERATE_EXPORT_HEADER(
             AppositionSurfacePlugin
             BASE_NAME AppositionSurfacePlugin
             EXPORT_MACRO_NAME AppositionSurfacePlugin_EXPORT
             EXPORT_FILE_NAME AppositionSurfacePlugin_Export.h
  )
target_link_libraries(AppositionSurfacePlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(AppositionSurfacePlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

#if (BUILD_TESTING)
#
#  add_subdirectory(tests)
#
#endif()
