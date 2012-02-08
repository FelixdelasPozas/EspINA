add_subdirectory( EspinaIO )
# add_subdirectory(SegFileRemoteWriter)
# add_subdirectory( SegFileRemoteReader )
# add_subdirectory( SegmhaReader )

# find_package (ITK REQUIRED)
# include (${ITK_USE_FILE})
# 
# include_directories(
#   ${CAJAL_INCLUDE_DIR}/filters
#   ${ITK_VTK_GLUE}
#   ${ITK_DIR}/../
#   ${CMAKE_CURRENT_SOURCE_DIR} 
#   )
# 
# set (VTKFILTER_SRCS
# 	${CAJAL_INCLUDE_DIR}/filters/vtkImageLabelMapBlend.cpp
# 	${CAJAL_INCLUDE_DIR}/filters/vtkMorphologicalFeaturesFilter.cpp
# 	../../common/vtkSegmentationSelectionFilter.cpp
# 	../../common/vtkNonRotatingBoxRepresentation.cpp
# 	../../common/vtkNonRotatingBoxWidget.cpp
# 	vtkPVImageSlicer.cxx
#   )
# 
# set (PARAVIEW_ADDITIONAL_LIBRARIES
#   ${CAJAL_LIBRARIES}
#   ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
# )

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
# ADD_PARAVIEW_PLUGIN( 
# 	EspinaFilters "0.3" 
# 	REQUIRED_ON_SERVER
# 	SERVER_MANAGER_XML EspinaFilters.xml
# 	SERVER_MANAGER_SOURCES ${VTKFILTER_SRCS} 
# 	)

# if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
#   set_target_properties(EspinaFilters PROPERTIES COMPILE_FLAGS "-fPIC")
# endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
# 
# target_link_libraries (EspinaFilters ${PARAVIEW_ADDITIONAL_LIBRARIES} )
# 
# install (TARGETS EspinaFilters LIBRARY DESTINATION bin/plugins)
