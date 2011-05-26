# project (ESPINA_REMOTE_FILE_PLUGIN)

cmake_minimum_required(VERSION 2.6)

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  FileReader "1.0"
  SERVER_MANAGER_SOURCES vtkRemoteFileReader.cpp #vtkFileContent.cpp
  SERVER_MANAGER_XML RemoteFile.xml
#   SOURCES ${LOCAL_SOURCES}
#   GUI_RESOURCES RemoteFilel.qrc
  GUI_RESOURCE_FILES RemoteFileGUI.xml
)

#target_link_libraries(RemoteFile)