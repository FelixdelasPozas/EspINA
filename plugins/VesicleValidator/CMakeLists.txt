project (VESICLE_VALIDATOR)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_SOURCE_DIR}/frontend
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${ITK_VTK_GLUE}
  ${Boost_INCLUDE_DIRS}
  ${ITK_DIR}/../
  )

qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/VesicleValidator.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
  VesicleValidator.h
  VesicleValidatorSettings.h
  )

# This is a macro for adding QActionGroup subclasses automatically as toolbars.
ADD_PARAVIEW_ACTION_GROUP(
  IFACES IFACE_SRCS 
  CLASS_NAME VesicleValidator 
  GROUP_NAME "ToolBar/Vesicle Validator")

set (VTKFILTER_SRCS
  vtkCrossSource.cpp
)

set (PLUGIN_SRCS
  VesicleValidator.cpp
  VesicleValidatorFilter.cpp
  VesicleValidatorSettings.cpp
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
  VesicleValidatorPlugin
  "0.1" 
  SERVER_MANAGER_XML VesicleValidator.xml
  SERVER_MANAGER_SOURCES ${VTKFILTER_SRCS}
  GUI_INTERFACES ${IFACES} 
  GUI_SOURCES ${IFACE_SRCS} ${PLUGIN_SRCS}
)

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(VesicleValidatorPlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (VesicleValidatorPlugin ${PARAVIEW_ADDITIONAL_LIBRARIES} )

install (TARGETS VesicleValidatorPlugin LIBRARY DESTINATION bin/plugins)

# add_subdirectory( tests )
