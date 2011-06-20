project (ESPINA_COUNTING_PLUGIN)

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
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  system filesystem)
link_directories ( ${Boost_LIBRARY_DIRS} )

foreach(FILE 
  vtkBoundingRegionFilter.h
  vtkBoundingRegionFilter.cpp
  vtkCountingRegionFilter.h
  vtkCountingRegionFilter.cpp
)
  add_custom_command(
    OUTPUT ${FILE}
    COMMAND ${CMAKE_COMMAND}
    ARGS -E create_symlink ${CAJAL_INCLUDE_DIR}/filters/${FILE} ${FILE}
    DEPENDS ${CAJAL_LIBRARIES}
    COMMENT "Creating symbolic link for ${FILE}"
  )
endforeach(FILE)

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${ITK_VTK_GLUE}
  ${Boost_INCLUDE_DIRS}
  ${ITK_DIR}/../
  )


qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/CountingRegion.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
  CountingRegion.h
  RegionRenderer.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  CountingRegionPanel.ui
  )

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_DOCK_WINDOW(
  IFACES IFACE_SRCS
  CLASS_NAME CountingRegion 
  DOCK_AREA Left)

file ( GLOB LOCAL_SOURCES *.cpp )

set (PLUGIN_SRCS
  ${LOCAL_SOURCES}
  ${MOC_SRCS} 
  ${RCC_BUILT_SOURCES} 
  ${UI_BUILT_SOURCES}
  )

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${PARAVIEW_ADDITIONAL_LIBRARIES}
  ${CAJAL_LIBRARIES}
  Espina
  ITKCommon ITKBasicFilters ITKAlgorithms 
  ${Boost_LIBRARIES}
)

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
  CountingRegion
  "0.1" 
  SERVER_MANAGER_XML CountingRegionFilters.xml
  SERVER_MANAGER_SOURCES vtkBoundingRegionFilter.h vtkBoundingRegionFilter.cpp 
                         vtkCountingRegionFilter.h vtkCountingRegionFilter.cpp
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
  set_target_properties(CountingRegion PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (CountingRegion ${PARAVIEW_ADDITIONAL_LIBRARIES} )
