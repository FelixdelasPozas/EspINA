project (ESPINA)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package(Cajal REQUIRED)
include(${CAJAL_USE_FILE})

add_subdirectory (common)
add_subdirectory (plugins)
add_subdirectory (frontend)
