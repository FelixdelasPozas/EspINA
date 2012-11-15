project(APPOSITION_PLANE_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

IF (NOT COMPILING_ESPINA)
  find_package(Espina REQUIRED)
  include(${ESPINA_USE_FILE})
ENDIF (NOT COMPILING_ESPINA)

include_directories (
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} 
)
qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/AppositionPlane.qrc
  )

QT4_WRAP_CPP(MOC_SOURCES
  AppositionPlane.h
)

set (PLUGIN_SOURCES 
  AppositionPlane.cpp
  AppositionPlaneExtension.cpp
  AppositionPlaneRenderer.cpp
  ${MOC_SOURCES}
  ${RCC_BUILT_SOURCES}
)

set (PLUGIN_LIBRARIES
  EspinaCore
  )

add_library(AppositionPlanePlugin SHARED ${PLUGIN_SOURCES})
target_link_libraries(AppositionPlanePlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(AppositionPlanePlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
#add_subdirectory(tests)
