project(APPOSITION_PLANE_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

if (NOT COMPILING_ESPINA)

  set (BUILD_TESTING OFF  CACHE BOOL "Build EspINA Unitary Tests")

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
  Analysis/SynapticAppositionSurfaceAnalysis.ui
  GUI/Settings/AppositionSurfaceSettings.ui
  GUI/FilterInspector/AppositionSurfaceFilterInspector.ui
  )

QT4_WRAP_CPP(MOC_SOURCES
  AppositionSurfacePlugin.h
  Analysis/SynapticAppositionSurfaceAnalysis.h
  Filter/AppositionSurfaceFilter.h

  GUI/Settings/AppositionSurfaceSettings.h
  GUI/FilterInspector/AppositionSurfaceFilterInspector.h
  GUI/AppositionSurfaceAction.h
  GUI/AppositionSurfaceToolbar.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
)

set (PLUGIN_SOURCES 
  ${MOC_SOURCES}
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  Analysis/SynapticAppositionSurfaceAnalysis.cpp
  AppositionSurfacePlugin.cpp
  Core/ASMeshProxy.cpp 
  Core/ASVolumeProxy.cpp
  Core/Extensions/AppositionSurfaceExtension.cpp
  Filter/AppositionSurfaceFilter.cpp
  GUI/AppositionSurfaceAction.cpp
  GUI/AppositionSurfaceToolbar.cpp
  GUI/FilterInspector/AppositionSurfaceFilterInspector.cpp
  GUI/Settings/AppositionSurfaceSettings.cpp
  Undo/AppositionSurfaceCommand.cpp
)

set (PLUGIN_LIBRARIES
  EspinaCore
  EspinaGUI
  EspinaUndo
  vtkInfovis
  ${XLSLIB_LIBRARIES}
  )

add_library(AppositionSurfacePlugin SHARED ${PLUGIN_SOURCES})
target_link_libraries(AppositionSurfacePlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(AppositionSurfacePlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

#if (BUILD_TESTING)
#
#  add_subdirectory(tests)
#
#endif()
