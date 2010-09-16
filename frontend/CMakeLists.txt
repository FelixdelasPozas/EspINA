project (ESPINA_FRONT)

cmake_minimum_required (VERSION 2.6)

find_package (Qt4 REQUIRED)

if (QT4_FOUND)
   include (${QT_USE_FILE})
endif (QT4_FOUND)

add_subdirectory (src)


