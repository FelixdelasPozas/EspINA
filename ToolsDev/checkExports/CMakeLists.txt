#
# Exmples
#
project(checkExports)

cmake_minimum_required (VERSION 3.1.0)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)
#include(${QT_USE_FILE})

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${QT_TEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

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
  Qt5::Core
  )

add_executable(checkExports ${SOURCES})
target_link_libraries (checkExports ${EXTERNAL_LIBS})