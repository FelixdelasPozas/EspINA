# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN(
	EspinaFilters
	"0.1" 
	SERVER_MANAGER_XML EspinaFilters.xml
	#SERVER_MANAGER_SOURCES vtkConnectedThresholdImageFilter.cpp
	#According to: http://markmail.org/message/wj27wmlj6rjd7vg5
	# - See the macro documentation in ParaView/CMake/ParaViewPlugins.cmake:~835
	# - the parser doesn't work because it seems there is no vtkClass inside
	#   thus we have to use a vtkXXXX class name for our filter, other wise,
	#   we need to use SERVER_SOURCES instead of SERVER_MANAGER_SOURCES
	#   SERVER_SOURCES SegmentationFilter.cpp 
	#SOURCES ${IFACE_SRCS} ${RCC_BUILT_SOURCES} ${PLUGIN_SRCS}
	#GUI_INTERFACES ${IFACES} 
	)
