find_package(Boost REQUIRED COMPONENTS  system filesystem graph)

# link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories(
  ${CMAKE_SOURCE_DIR}/common 
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
#   ${Boost_INCLUDE_DIRS}
#   ${ITK_DIR}/../
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (MOC_BUILT_SOURCES
  EspinaWindow.h
  MainToolBar.h
  SegmentationExplorer.h
  docks/TaxonomyInspector.h
  toolbar/LODToolBar.h
#   sampleEditor.h
#   SegmentationExplorer.h
#   qTreeComboBox.h
#   preferencesDialog.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  SegmentationExplorer.ui
  docks/TaxonomyInspector.ui
#   EspinaWindow.ui
#   sampleEditor.ui
#   segmentationExplorer.ui
#   preferencesDialog.ui
#   GeneralPreferences.ui
#   ViewPreferences.ui
  )

# source_group ("Generated" FILES 
#   ${MOC_BUILT_SOURCES}
#   ${UI_BUILT_SOURCES}
#   ${RCC_BUILT_SOURCES}
#   )

set (DOCKS_SOURCES
  docks/TaxonomyInspector.cpp
  )

set (LOCAL_SOURCES
  EspinaWindow.cpp
  MainToolBar.cpp
  toolbar/LODToolBar.cpp
  SegmentationExplorer.cpp
  )

set (Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${DOCKS_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set (Espina_LIBRARIES
  EspinaCore
  EspinaViews
#   ${Boost_LIBRARIES}
)


#------------------------------------------------------------------------------
# Build the client
build_paraview_client(espina
  APPLICATION_NAME "EspINA"
  TITLE "EspINA"
  ORGANIZATION  "CeSViMa"
  VERSION_MAJOR ${ESPINA_VERSION_MAJOR}
  VERSION_MINOR ${ESPINA_VERSION_MINOR}
  VERSION_PATCH ${ESPINA_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/rsc/SplashScreen.png"
  PVMAIN_WINDOW EspinaWindow
  PVMAIN_WINDOW_INCLUDE EspinaWindow.h
#   BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/rsc/MacIcon.icns"
#   APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/rsc/WinIcon.ico"
  GUI_CONFIGURATION_XMLS
  ${CMAKE_CURRENT_SOURCE_DIR}/rsc/EspinaReaders.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/rsc/EspinaWriters.xml
  #${PARAVIEW_EXTRA_GUI_RESOURCES} # Add any compiled in GUI configuration xmls
  #COMPRESSED_HELP_FILE "${ParaView_BINARY_DIR}/Documentation/paraview.qch"
  SOURCES ${Espina_SOURCE_FILES}
	#INSTALL_BIN_DIR "${PV_INSTALL_BIN_DIR}"
	#INSTALL_LIB_DIR "${PV_INSTALL_LIB_DIR}"
  )

target_link_libraries (espina ${Espina_LIBRARIES})

install (
  TARGETS espina
  RUNTIME
  DESTINATION bin
  COMPONENT espina
  )


add_subdirectory (plugins)
