project (COUNTING_REGION_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 2)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required (VERSION 2.6)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Espina REQUIRED)
include(${ESPINA_USE_FILE})

include_directories(
  ${CMAKE_SOURCE_DIR} #TODO: change to use espina path
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # UI files
  )


qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/CountingRegion.qrc
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  CountingRegion.ui
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
  CountingRegion.h
  extensions/CountingRegionSegmentationExtension.h
  regions/BoundingRegion.h
  )


# file ( GLOB LOCAL_SOURCES *.cpp )
set (LOCAL_SOURCES
  CountingRegion.cpp
  extensions/CountingRegionSampleExtension.cpp
  extensions/CountingRegionSegmentationExtension.cpp
  )

set (REGION_SOURCES
  regions/AdaptiveBoundingRegion.cpp
  regions/BoundingRegion.cpp
  regions/RectangularBoundingRegion.cpp
  regions/vtkBoundingRegionSliceRepresentation.cpp
  regions/vtkBoundingRegionSliceWidget.cpp
  regions/vtkBoundingRegion3DRepresentation.cpp
  regions/vtkBoundingRegion3DWidget.cpp
  )

set (PLUGIN_SOURCES
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${MOC_SRCS}
  ${LOCAL_SOURCES}
  ${REGION_SOURCES}
  )

set (PLUGIN_LIBRARIES
  EspinaCore
  )

add_library(CountingRegionPlugin SHARED ${PLUGIN_SOURCES})
target_link_libraries (CountingRegionPlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(CountingRegionPlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )


# install(
#   TARGETS CountingRegion
#   LIBRARY
#   DESTINATION bin/plugins
#   #COMPONENT CountingRegion
#   )