FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

QT4_ADD_RESOURCES(RCC_BUILT_SOURCES ../rsc/segmentation.qrc)

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
QT4_WRAP_CPP(MOC_SRCS SegmentationToolbarActions.h)

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(IFACES IFACE_SRCS 
                          CLASS_NAME SegmentationToolbarActions
                          GROUP_NAME "ToolBar/Segmentation")

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
	Segmentation 
	"0.1" 
	GUI_INTERFACES ${IFACES} 
	SOURCES ${MOC_SRCS} ${IFACE_SRCS} ${RCC_BUILT_SOURCES} SegmentationToolbarActions.cpp
	SERVER_MANAGER_XML MyElevationFilter.xml
	SERVER_MANAGER_SOURCES vtkMyElevationFilter.cxx
	)
