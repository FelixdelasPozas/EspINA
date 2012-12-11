qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/VolumeOfInterest.qrc
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS
  VolumeOfInterest.h
#   pqNonRotatingBoxWidget.h
#   VolumeOfInterestPreferences.h
#   RectangularVOI.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  SettingsPanel.ui
)

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(VolumeOfInterest PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

target_link_libraries (VolumeOfInterest ${PARAVIEW_ADDITIONAL_LIBRARIES} )

# install (
#   TARGETS VolumeOfInterest
#   LIBRARY
#   DESTINATION bin/plugins
#   #COMPONENT VolumeOfInterest
# )
