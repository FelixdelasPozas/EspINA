#
# Core tools
#
project(FakeStacks)

cmake_minimum_required (VERSION 2.8.6)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CORE_EXTERNAL_LIBS ${CORE_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
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

# Add Qt Resource files
qt4_add_resources(RESOURCES
  rsc/resources.qrc
  )

qt4_wrap_ui(CORE_UI
  # .ui para Qt
  FakeStacks.ui
  )
	
qt4_wrap_cpp(CORE_MOC
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
