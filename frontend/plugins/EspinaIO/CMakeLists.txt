find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  EspinaIO "1.0"
  SERVER_MANAGER_SOURCES vtkSegReader.cpp vtkSegWriter.cpp
  SERVER_MANAGER_XML EspinaIO.xml
  GUI_RESOURCE_FILES EspinaIO_GUI.xml
  GUI_SOURCES FilePack.cpp
)

# target_link_libraries(FileReader Espina) #Required to be usedin server mode
# I think it's no more required because we have moved FilePack inside the library

install (TARGETS EspinaIO
         LIBRARY
         DESTINATION bin/plugins
         COMPONENT espinaApp)
