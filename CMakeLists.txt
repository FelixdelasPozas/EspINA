project (ESPINA)

cmake_minimum_required (VERSION 2.6)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package(Cajal REQUIRED)

add_subdirectory (common)
add_subdirectory (plugins)
add_subdirectory (frontend)