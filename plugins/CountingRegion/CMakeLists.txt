project (COUNTING_REGION_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required (VERSION 2.6)

find_package (ParaView REQUIRED 1.12) # Include Qt4
include (${PARAVIEW_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

include_directories(
  ${CMAKE_SOURCE_DIR} #TODO: change to use espina path
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/server
  ${ITK_VTK_GLUE}
  )


qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/CountingRegion.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
  CountingRegion.h
#   regions/BoundingRegion.h
#   regions/RectangularRegion.h
#   CountingRegionExtension.h
#   RegionRenderer.h
  pqRectangularBoundingRegionWidget.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  CountingRegion.ui
  )

ADD_3DWIDGET(
  WIDGET_IFACES WIDGET_SOURCES
  CLASS_NAME pqRectangularBoundingRegionWidget
  WIDGET_TYPE RectangularBoundingRegion
  )

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_DOCK_WINDOW(
  DOCK_IFACES DOCK_SOURCES
  CLASS_NAME CountingRegion 
  DOCK_AREA Left)

# file ( GLOB LOCAL_SOURCES *.cpp )
set (LOCAL_SOURCES
  CountingRegion.cpp
  pqRectangularBoundingRegionWidget.cpp
  )

set (REGION_SOURCES
  regions/BoundingRegion.cpp
  regions/RectangularBoundingRegion.cpp
  )

set (SERVER_SOURCES
  server/vtkAdaptiveBoundingRegionFilter.cpp
  server/vtkRectangularBoundingRegionFilter.cpp
  server/vtkCountingRegionFilter.cpp
  server/vtkRectangularBoundingRegionRepresentation.cpp
  server/vtkRectangularBoundingRegionWidget.cpp
)

set (PLUGIN_SRCS
  ${LOCAL_SOURCES}
  ${REGION_SOURCES}
  ${MOC_SRCS} 
  ${RCC_BUILT_SOURCES} 
  ${UI_BUILT_SOURCES}
  ${WIDGET_SOURCES}
  ${DOCK_SOURCES}
  )

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  EspinaCore
  ITKCommon ITKBasicFilters ITKAlgorithms 
)

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
  CountingRegion
  "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}"
  SERVER_MANAGER_XML CountingRegionFilters.xml
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp 
  SOURCES ${PLUGIN_SRCS}
  GUI_INTERFACES ${DOCK_IFACES} ${WIDGET_IFACES}
  )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(CountingRegion PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (CountingRegion ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install(
  TARGETS CountingRegion
  LIBRARY
  DESTINATION bin/plugins
  COMPONENT CountingRegion
  )
