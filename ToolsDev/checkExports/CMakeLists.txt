#
# Exmples
#
project(checkExports)

cmake_minimum_required (VERSION 2.8.6)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}          # Generated .h files
  ${CMAKE_CURRENT_BINARY_DIR}  # For wrap/ui files
  )

set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -std=c++11 -fpermissive")

set (SOURCES
  main.cpp
)

set (EXTERNAL_LIBS
  ${EXTERNAL_LIBS}
  QtCore
  )

add_executable(checkExports ${SOURCES})
target_link_libraries (checkExports ${EXTERNAL_LIBS})