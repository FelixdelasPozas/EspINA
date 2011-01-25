project (ESPINA_SEEDGROW_PLUGIN)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)

#TODO: load only qt  
if (QT4_FOUND)
   include (${QT_USE_FILE})
endif (QT4_FOUND)


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

SET (ITK_VTK_GLUE "" CACHE PATH "Path to ItkVtkGlue external project")

FIND_PACKAGE(Cajal REQUIRED)

include_directories(
	../../common
	${CAJAL_INCLUDE_DIR}/data
	${CAJAL_INCLUDE_DIR}/filters
	${ITK_VTK_GLUE}
	)


QT4_ADD_RESOURCES(RCC_BUILT_SOURCES 
	rsc/segmentation.qrc)

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
QT4_WRAP_CPP(MOC_SRCS 
	SeedGrowingSegmentation.h
	)

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(IFACES IFACE_SRCS 
                          CLASS_NAME SeedGrowingSegmentation
                          GROUP_NAME "ToolBar/SeedGrowingSegmentation")

FILE ( GLOB LOCAL_SOURCES *.cpp )

SET(PLUGIN_SRCS
	${LOCAL_SOURCES}
	${MOC_SRCS} 
	)

SET (PARAVIEW_ADDITIONAL_LIBRARIES
	     ${PARAVIEW_ADDITIONAL_LIBRARIES}
		 ${CAJAL_LIBRARIES}
		 Espina
		 ITKCommon ITKBasicFilters ITKAlgorithms 
		 )

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
	Segmentation
	"0.1" 
	SERVER_MANAGER_XML SeedGrowSegmentation.xml
	SERVER_MANAGER_SOURCES vtkConnectedThresholdImageFilter.cpp
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp 
	SOURCES ${IFACE_SRCS} ${RCC_BUILT_SOURCES} ${PLUGIN_SRCS}
	GUI_INTERFACES ${IFACES} 
	)

target_link_libraries (Segmentation ${PARAVIEW_ADDITIONAL_LIBRARIES})
