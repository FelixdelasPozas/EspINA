project (ESPINA_FRONTEND)

cmake_minimum_required (VERSION 2.6)

#find_package(Cajal REQUIRED)

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)

link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories(
  ${CMAKE_SOURCE_DIR}/common 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  ${CAJAL_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS} 
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES
  rsc/espina.qrc
  )

# Create header's MOC files
qt4_wrap_cpp (MOC_BUILT_SOURCES
  espinaMainWindow.h
  unitExplorer.h
  slicer.h
  sliceView.h
  volumeView.h
  qTreeComboBox.h
  Crosshairs.h
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  espinaMainWindow.ui
  unitExplorer.ui
  )


source_group ("Generated" FILES 
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

file ( GLOB LOCAL_SOURCES *.cpp )

set (Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set (Espina_LIBRARIES
  Espina
  ${Boost_LIBRARIES}
)


#------------------------------------------------------------------------------
# Build the client
build_paraview_client(espina
  APPLICATION_NAME "ESPINA"
  TITLE "Espina"
  ORGANIZATION  "CesViMa"
  VERSION_MAJOR 0
  VERSION_MINOR 1
  VERSION_PATCH 0
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/rsc/SplashScreen.png"
  PVMAIN_WINDOW EspinaMainWindow
  PVMAIN_WINDOW_INCLUDE espinaMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/rsc/MacIcon.icns"
  APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/rsc/WinIcon.ico"
  GUI_CONFIGURATION_XMLS
	#  ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewSources.xml
	#  ${CMAKE_CURRENT_SOURCE_DIR}/plugins/EspinaFilters.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/rsc/EspinaReaders.xml
	#  ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewWriters.xml
	#  ${PARAVIEW_EXTRA_GUI_RESOURCES} # Add any compiled in GUI configuration xmls
  COMPRESSED_HELP_FILE "${ParaView_BINARY_DIR}/Documentation/paraview.qch"
  SOURCES ${Espina_SOURCE_FILES}
	#INSTALL_BIN_DIR "${PV_INSTALL_BIN_DIR}"
	#INSTALL_LIB_DIR "${PV_INSTALL_LIB_DIR}"
  )

target_link_libraries (espina ${Espina_LIBRARIES})

add_subdirectory (plugins)
