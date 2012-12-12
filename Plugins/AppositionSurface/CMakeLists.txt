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

include_directories (
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/AppositionSurface.qrc
  )

QT4_WRAP_CPP(MOC_SOURCES
  AppositionSurface.h
)

set (PLUGIN_SOURCES
  AppositionSurface.cpp
  AppositionSurfaceExtension.cpp
  AppositionSurfaceRenderer.cpp
  ${MOC_SOURCES}
  ${RCC_BUILT_SOURCES}
)

set (PLUGIN_LIBRARIES
  EspinaCore
  vtkInfovis
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
