project (COUNTING_REGION_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 2)
set (PLUGIN_VERSION_MINOR 0)
set (PLUGIN_VERSION_PATCH 1)

include (GenerateExportHeader)

cmake_minimum_required (VERSION 3.1.0)

find_package(XLSLIB REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)
#include(${QT_USE_FILE})

find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

IF (NOT COMPILING_ESPINA)
  find_package(Espina REQUIRED)
  include(${ESPINA_USE_FILE})
ENDIF (NOT COMPILING_ESPINA)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # UI files
  ${XLSLIB_INCLUDE_DIRS}
  )

qt5_add_resources (RCC_BUILT_SOURCES
  rsc/CountingFrame.qrc
  )

# Create Interface files
qt5_wrap_ui (UI_BUILT_SOURCES
  Dialogs/CFTypeSelectorDialog.ui
  Panel.ui
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt5_wrap_cpp(MOC_SRCS
  ColorEngines/ColorEngine.h
  ColorEngines/ColorEngineSwitch.h
  CountingFrameManager.h
  CountingFramePlugin.h
  CountingFrames/CountingFrame.h
  Dialogs/CFTypeSelectorDialog.h
  Extensions/CountingFrameExtension.h
  Extensions/StereologicalInclusion.h
  Panel.h
  Tasks/ApplyCountingFrame.h
  Tasks/CountingFrameCreator.h
  Representations/RepresentationManager2D.h
  Representations/RepresentationManager3D.h
  Representations/RepresentationSwitch.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (LOCAL_SOURCES
  ColorEngines/ColorEngine.cpp
  ColorEngines/ColorEngineSwitch.cpp
  CountingFrameManager.cpp
  CountingFramePlugin.cpp
  CountingFrames/AdaptiveCountingFrame.cpp
  CountingFrames/CountingFrame.cpp
  CountingFrames/OrthogonalCountingFrame.cpp
  CountingFrames/vtkCountingFrame3DRepresentation.cpp
  CountingFrames/vtkCountingFrame3DWidget.cpp
  CountingFrames/vtkCountingFrameRepresentationXY.cpp
  CountingFrames/vtkCountingFrameRepresentationXZ.cpp
  CountingFrames/vtkCountingFrameRepresentationYZ.cpp
  CountingFrames/vtkCountingFrameSliceRepresentation.cpp
  CountingFrames/vtkCountingFrameSliceWidget.cpp
  Dialogs/CFTypeSelectorDialog.cpp
  Extensions/CountingFrameExtension.cpp
  Extensions/CountingFrameFactories.cpp
  Extensions/StereologicalInclusion.cpp
  Representations/RepresentationFactory.cpp
  Representations/RepresentationManager2D.cpp
  Representations/RepresentationManager3D.cpp
  Representations/RepresentationSwitch.cpp
  Tasks/ApplyCountingFrame.cpp
  Tasks/CountingFrameCreator.cpp
  Panel.cpp
  )

set (PLUGIN_SOURCES
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${MOC_SRCS}
  ${LOCAL_SOURCES}
  )

set (PLUGIN_LIBRARIES
  EspinaCore
  EspinaGUI
  EspinaUndo
  EspinaSupport
  )

if(DEFINED MINGW OR DEFINED MSVC)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW OR DEFINED MSVC)

add_library(CountingFramePlugin SHARED ${PLUGIN_SOURCES})
GENERATE_EXPORT_HEADER(
             CountingFramePlugin
             BASE_NAME CountingFramePlugin
             EXPORT_MACRO_NAME CountingFramePlugin_EXPORT
             EXPORT_FILE_NAME CountingFramePlugin_Export.h
  )
  
#if(DEFINED MINGW OR DEFINED MSVC)
#  SET(PLUGIN_LIBRARIES ${PLUGIN_LIBRARIES} iconv)
#endif(DEFINED MINGW OR DEFINED MSVC)

target_link_libraries (CountingFramePlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(CountingFramePlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

install(
  TARGETS CountingFramePlugin
  DESTINATION ${INSTALL_BIN_DIR}/plugins
  )
