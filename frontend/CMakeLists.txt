project (ESPINA_FRONTEND)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)

if (QT4_FOUND)
   include (${QT_USE_FILE})
endif (QT4_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

add_subdirectory (src)
