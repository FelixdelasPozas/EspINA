project( SEED_GROW_SEGMENTATION_PLUGIN )

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
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${ITK_VTK_GLUE}
#   ${ITK_DIR}/../
  )


qt4_add_resources (RCC_BUILT_SOURCES 
#   rsc/SeedGrowSegmentation.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
#   SeedGrowSegmentation.h
#   SeedGrowSegmentationPreferences.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
#   SeedGrowSegmentationFilterSetup.ui
#   SeedGrowSegmentationPreferences.ui
  )


# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(
  IFACES IFACE_SRCS 
  CLASS_NAME SeedGrowSegmentation 
  GROUP_NAME "ToolBar/SeedGrowingSegmentation")


set (PLUGIN_SRCS
  SeedGrowSegmentation.cpp
#   SeedGrowSegmentationPreferences.cpp
#   SeedGrowSegmentationFilter.cpp
  ${MOC_SRCS} 
  ${RCC_BUILT_SOURCES} 
  ${UI_BUILT_SOURCES} 
  )

set (VTKFILTER_SRCS
  vtkConnectedThresholdImageFilter.cpp
  )

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  Espina
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
#   ${Boost_LIBRARIES}
)

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
  SeedGrowSegmentationPlugin
  "0.1"
  SERVER_MANAGER_XML SeedGrowSegmentationFilters.xml
  SERVER_MANAGER_SOURCES ${VTKFILTER_SRCS}
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp 
  GUI_INTERFACES ${IFACES} 
  GUI_SOURCES ${IFACE_SRCS} ${PLUGIN_SRCS}
  )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(SeedGrowSegmentationPlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (SeedGrowSegmentationPlugin ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install (TARGETS SeedGrowSegmentationPlugin LIBRARY DESTINATION bin/plugins)

# if (UNIX)
#   set (CPACK_PACKAGE_VERSION_MAJOR ${ESPINA_VERSION_MAJOR})
#   set (CPACK_PACKAGE_VERSION_MINOR ${ESPINA_VERSION_MINOR})
#   set (CPACK_PACKAGE_VERSION_PATCH ${ESPINA_VERSION_PATCH})
#   set (CPACK_PACKAGE_NAME EspINA )
#   set (CPACK_GENERATOR "DEB")
#   set (CPACK_DEBIAN_PACKAGE_MAINTAINER "CeSViMa") #required
#   set (CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system1.42.0,
#         libboost-filesystem1.42.0,
#         libboost-graph1.42.0,
#         libqt4-webkit,
#         libqt4-svg,
#         libqt4-help,
#         libqt4-xmlpatterns,
#         libopenmpi1.3")
#   set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/CMake/debian/postinst;")
# 
#   include (CPack)
# endif (UNIX)