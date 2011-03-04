project (ESPINA_REMOTE_FILE_PLUGIN)
cmake_minimum_required(VERSION 2.6)

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

# find_package (Cajal REQUIRED)
# include (${ITK_USE_FILE})

# ADD_EXECUTABLE(AlgorithmSourceExample 
#   vtkFileContent.cpp
#   vtkRemoteFileReader.cpp
# 
# )


ADD_PARAVIEW_PLUGIN(RemoteFile "1.0" 
  SERVER_MANAGER_XML RemoteFile.xml
  SERVER_MANAGER_SOURCES vtkRemoteFileReader.cpp
  GUI_RESOURCE_FILES MyReaderGUI.xml)


############################
# SET (ITK_VTK_GLUE "" CACHE PATH "Path to ItkVtkGlue external project")


include_directories(
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR} 
#   ${ITK_VTK_GLUE}
#   ${Boost_INCLUDE_DIRS}
  )


# This is a macro for adding QActionGroup subclasses automatically as toolbars.
# ADD_PARAVIEW_ACTION_GROUP(
#   IFACES IFACE_SRCS 
#   CLASS_NAME SeedGrowingSegmentation 
#   GROUP_NAME "ToolBar/SeedGrowingSegmentation")
 
TARGET_LINK_LIBRARIES(RemoteFile vtkHybrid)