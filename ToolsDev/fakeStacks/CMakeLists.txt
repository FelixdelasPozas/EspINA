#
# Core tools
#
project(FakeStacks)

cmake_minimum_required (VERSION 3.1.0)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)
#include(${QT_USE_FILE})

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_TEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}          # Generated .h files
  ${CMAKE_CURRENT_BINARY_DIR}  # For wrap/ui files
  )

set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -std=c++11 -fpermissive")

# Add Qt Resource files
qt5_add_resources(RESOURCES
  rsc/resources.qrc
  )

qt5_wrap_ui(CORE_UI
  # .ui para Qt
  FakeStacks.ui
  )
	
qt5_wrap_cpp(CORE_MOC
  # .h para Qt
  FakeStacks.h
  )

set (CORE_SOURCES
  # ficheros para compilar
  ${RESOURCES}
  ${CORE_MOC}
  ${CORE_UI}
  main.cpp
  FakeStacks.cpp
)

set(CORE_EXTERNAL_LIBS
  EspinaCore
  EspinaFilters
  EspinaGUI
  EspinaSupport
  EspinaUndo
)

add_executable(fakeStacks ${CORE_SOURCES})
target_link_libraries (fakeStacks ${CORE_EXTERNAL_LIBS})
