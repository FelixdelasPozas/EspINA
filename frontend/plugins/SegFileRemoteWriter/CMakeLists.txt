cmake_minimum_required(VERSION 2.6)

find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  SegFileWriter "2.0"
  SERVER_MANAGER_SOURCES vtkSegWriter.cpp
  SERVER_MANAGER_XML segWriter.xml
)

install (TARGETS SegFileWriter LIBRARY DESTINATION bin/plugins)
