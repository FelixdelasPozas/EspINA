project (ESPINA_SEEDGROWREGION_PLUGIN)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package (Cajal REQUIRED)
include (${CAJAL_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
link_directories ( ${Boost_LIBRARY_DIRS} )


SET (ITK_VTK_GLUE "" CACHE PATH "Path to ItkVtkGlue external project")


include_directories(
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${ITK_VTK_GLUE}
  ${Boost_INCLUDE_DIRS}
  ${ITK_DIR}/../
  )


qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/segmentationRegion.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS
  SeedGrowRegionSegmentation.h
  )

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(
  IFACES IFACE_SRCS 
  CLASS_NAME SeedGrowRegionSegmentation
  GROUP_NAME "ToolBar/SeedGrowRegionSegmentation")#"ToolBar/SeedGrowingRegionSegmentation")

file ( GLOB LOCAL_SOURCES *.cpp )

set (PLUGIN_SRCS
  ${LOCAL_SOURCES}
  ${MOC_SRCS} 
  ${RCC_BUILT_SOURCES} 
  )

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  ${CAJAL_LIBRARIES}
  Espina
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
  ${Boost_LIBRARIES}
)


# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
  SegmentationRegion
  "0.1"
  SERVER_MANAGER_XML SeedGrowRegionSegmentation.xml
  SERVER_MANAGER_SOURCES vtkConnectedThresholdRegionGrowImageFilter.cpp
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp
  SOURCES ${IFACE_SRCS} ${PLUGIN_SRCS}
  GUI_INTERFACES ${IFACES}
  )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(SegmentationRegion PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (SegmentationRegion ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install (TARGETS SegmentationRegion LIBRARY DESTINATION /usr/bin/plugins)
