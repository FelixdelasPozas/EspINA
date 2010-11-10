FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

FIND_PACKAGE(Cajal)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(
	${Cajal_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
QT4_ADD_RESOURCES(RCC_BUILT_SOURCES
	../rsc/espina.qrc)
# Add extra library containing custom code for the client.
QT4_WRAP_CPP(MOC_BUILT_SOURCES
espinaMainWindow.h)

QT4_WRAP_UI(UI_BUILT_SOURCES
  espinaMainWindow.ui
)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)


SET(Espina_SOURCE_FILES
  espinaMainWindow.cpp
  sliceWidget.cpp
  volumeWidget.cpp
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
    SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/SplashScreen.png"
    PVMAIN_WINDOW EspinaMainWindow
    PVMAIN_WINDOW_INCLUDE espinaMainWindow.h
    BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
    APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/WinIcon.ico"
	GUI_CONFIGURATION_XMLS
	#  ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewSources.xml
	#  ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewFilters.xml
	  ${CMAKE_CURRENT_SOURCE_DIR}/EspinaReaders.xml
	#  ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewWriters.xml
	#  ${PARAVIEW_EXTRA_GUI_RESOURCES} # Add any compiled in GUI configuration xmls
	COMPRESSED_HELP_FILE "${ParaView_BINARY_DIR}/Documentation/paraview.qch"
    SOURCES ${Espina_SOURCE_FILES}
	#INSTALL_BIN_DIR "${PV_INSTALL_BIN_DIR}"
	#INSTALL_LIB_DIR "${PV_INSTALL_LIB_DIR}"
)

target_link_libraries (espina ${Cajal_LIBRARIES})
