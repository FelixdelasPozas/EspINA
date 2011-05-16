cmake_minimum_required(VERSION 2.6)

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  SegFileWriter "1.0"
  SERVER_MANAGER_SOURCES vtkSegWriter.cpp
  SERVER_MANAGER_XML segWriter.xml
)
