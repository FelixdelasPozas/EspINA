#
# Exmples
#
project(information)

cmake_minimum_required (VERSION 2.8.6)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

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
