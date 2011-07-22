
add_subdirectory(SegFileRemoteWriter)
add_subdirectory( SegFileRemoteReader )

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

include_directories(
  ${ITK_VTK_GLUE}
  ${ITK_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR} 
  )

set (PLUGIN_SRCS
  vtkImageLabelMapBlend.h
  vtkImageLabelMapBlend.cpp
  vtkMorphologicalFeaturesFilter.h
  vtkMorphologicalFeaturesFilter.cpp
  )

foreach(FILE 
		${PLUGIN_SRCS}
)
  add_custom_command(
    OUTPUT ${FILE}
    COMMAND ${CMAKE_COMMAND}
    ARGS -E create_symlink ${CAJAL_INCLUDE_DIR}/filters/${FILE} ${FILE}
    DEPENDS ${CAJAL_LIBRARIES}
    COMMENT "Creating symbolic link for ${FILE}"
  )
endforeach(FILE)

set (PARAVIEW_ADDITIONAL_LIBRARIES
  ${CAJAL_LIBRARIES}
  ITKCommon ITKBasicFilters ITKAlgorithms ITKIO
)

# Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
# which are filled up by the above macro with relevant entries
ADD_PARAVIEW_PLUGIN( 
	EspinaFilters "0.2" 
	REQUIRED_ON_SERVER
	SERVER_MANAGER_XML EspinaFilters.xml
	SERVER_MANAGER_SOURCES ${PLUGIN_SRCS} 
	)

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(EspinaFilters PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (EspinaFilters ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install (TARGETS EspinaFilters LIBRARY DESTINATION /usr/bin/plugins)
