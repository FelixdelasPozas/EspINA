project (ESPINA_FRONTEND)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)
if (QT4_FOUND)
	include (${QT_USE_FILE})
endif (QT4_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package(Cajal REQUIRED)

find_package(Boost REQUIRED COMPONENTS  system filesystem)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

include_directories(
	.
	../common
	${CAJAL_INCLUDE_DIR}/data
	${CMAKE_CURRENT_BINARY_DIR}
	)

#------------------------------------------------------------------------------
# Add Qt Resource files
QT4_ADD_RESOURCES(RCC_BUILT_SOURCES
	rsc/espina.qrc)

# Create header's MOC files
QT4_WRAP_CPP(MOC_BUILT_SOURCES
	espinaMainWindow.h
	unitExplorer.h
	sliceWidget.h
	slicer.h
	volumeWidget.h
	)

# Create Interface files
QT4_WRAP_UI(UI_BUILT_SOURCES
	espinaMainWindow.ui
	unitExplorer.ui
	)


SOURCE_GROUP("Generated" FILES
	${MOC_BUILT_SOURCES}
	${UI_BUILT_SOURCES}
	${RCC_BUILT_SOURCES}
	)

FILE ( GLOB LOCAL_SOURCES *.cpp )

SET(Espina_SOURCE_FILES
	${LOCAL_SOURCES}
	${MOC_BUILT_SOURCES}
	${UI_BUILT_SOURCES}
	${RCC_BUILT_SOURCES}
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

target_link_libraries (espina Espina ${Boost_LIBRARIES})

add_subdirectory (plugins)
