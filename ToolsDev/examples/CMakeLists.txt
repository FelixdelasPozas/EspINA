#
# Exmples
#
project(information)

cmake_minimum_required (VERSION 3.1.0)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)
#include(${QT_USE_FILE})

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_TEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -std=c++11 -fpermissive")

set (SOURCES
  main.cpp
)

set(EXTERNAL_LIBS
  EspinaCore
)

add_executable(info ${SOURCES})
target_link_libraries (info ${EXTERNAL_LIBS})
