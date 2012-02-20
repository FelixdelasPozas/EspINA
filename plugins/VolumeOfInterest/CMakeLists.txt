project (VOLUME_OF_INTERESET_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 1)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required (VERSION 2.6)

find_package (ParaView REQUIRED 1.12) # Include Qt4
include (${PARAVIEW_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

# find_package (ITK REQUIRED)
# include (${ITK_USE_FILE})

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
#   ${ITK_VTK_GLUE}
  )

qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/VolumeOfInterest.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS
  VolumeOfInterest.h
#   pqNonRotatingBoxWidget.h
#   VolumeOfInterestPreferences.h
#   RectangularVOI.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
#   pqNonRotatingBoxWidget.ui
#   VOIPreferences.ui
)

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(
  IFACES IFACE_SRCS 
  CLASS_NAME VolumeOfInterest 
  GROUP_NAME "ToolBar/VolumeOfInterest")

set (PLUGIN_SOURCES
  VolumeOfInterest.cpp
  )

set (PLUGIN_SRCS
  ${PLUGIN_SOURCES}
  ${MOC_SRCS} 
  ${RCC_BUILT_SOURCES} 
  ${UI_BUILT_SOURCES}
  )

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  EspinaCore
  EspinaWidgets
)

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
  VolumeOfInterest
  "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}"
#   SERVER_MANAGER_XML VolumeOfInterestFilters.xml
  #SERVER_MANAGER_SOURCES vtkConnectedThresholdImageFilter.cpp
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp 
  SOURCES ${IFACE_SRCS} ${PLUGIN_SRCS} ${OUTSRCS}
  GUI_INTERFACES ${IFACES} ${OUTIFACES}
  )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(VolumeOfInterest PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (VolumeOfInterest ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install (
  TARGETS VolumeOfInterest
  LIBRARY
  DESTINATION bin/plugins
  COMPONENT VolumeOfInterest
)
