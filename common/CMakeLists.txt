find_package (Qt4 REQUIRED)

if (QT4_FOUND)
   include (${QT_USE_FILE})
endif (QT4_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

add_subdirectory (src)
