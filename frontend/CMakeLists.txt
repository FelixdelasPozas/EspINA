find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

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
  PreferencesDialog.h
  docks/DataView/DataView.h
  docks/DataView/QueryView.h
  docks/DataView/InformationProxy.h
  docks/ModifyFilter/ModifyFilterPanel.h
  docks/SegmentationExplorer.h
  docks/TaxonomyInspector.h
  toolbar/MainToolBar.h
  toolbar/LODToolBar.h
  views/DefaultEspinaView.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  docks/DataView/DataView.ui
  docks/DataView/QueryView.ui
  docks/SegmentationExplorer.ui
  docks/TaxonomyInspector.ui
  GeneralPreferences.ui
  PreferencesDialog.ui
  )

set (DOCKS_SOURCES
  docks/DataView/DataView.cpp
  docks/DataView/DataViewPanel.cpp
  docks/DataView/QueryView.cpp
  docks/DataView/InformationProxy.cpp
  docks/ModifyFilter/ModifyFilterPanel.cpp
  docks/SegmentationExplorer.cpp
  docks/TaxonomyInspector.cpp
  )

set (TOOLBAR_SOURCES
  toolbar/MainToolBar.cpp
  toolbar/LODToolBar.cpp
  )

set (VIEW_SOURCES
  views/DefaultEspinaView.cpp
  )

set (LOCAL_SOURCES
  EspinaWindow.cpp
  PreferencesDialog.cpp
  RecentDocuments.cpp
  )

set (Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${DOCKS_SOURCES}
  ${VIEW_SOURCES}
  ${TOOLBAR_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set (Espina_LIBRARIES
  EspinaCore
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

#install (
#  TARGETS espina
#  RUNTIME
#  DESTINATION bin
#  COMPONENT espina
#  )


add_subdirectory (plugins)
