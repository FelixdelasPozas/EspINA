find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

ADD_PARAVIEW_AUTO_START(IFACES IFACE_SOURCES
  CLASS_NAME EspinaIO
  STARTUP    onStartup
  SHUTDOWN   onShutdown
)

set (SERVER_SOURCES
  vtkSegReader.cpp
  vtkSegWriter.cpp
  )

set (PLUGIN_SOURCES
  EspinaIO.cpp
  FilePack.cpp
  )

ADD_PARAVIEW_PLUGIN(
  EspinaIO
  "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_MANAGER_XML EspinaIO.xml
  GUI_RESOURCE_FILES EspinaIO_GUI.xml
  SOURCES ${PLUGIN_SOURCES} ${IFACE_SOURCES}
  GUI_INTERFACES ${IFACES}
)

target_link_libraries(EspinaIO ${QUAZIP_LIBRARIES})#Espina) #Required to be usedin server mode
# I think it's no more required because we have moved FilePack inside the library

install (TARGETS EspinaIO
         LIBRARY
         DESTINATION bin/plugins
         COMPONENT espinaApp)
