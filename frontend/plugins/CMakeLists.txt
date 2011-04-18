# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN( 
	EspinaFilters "0.1" 
	REQUIRED_ON_SERVER
	SERVER_MANAGER_XML EspinaFilters.xml
	)
