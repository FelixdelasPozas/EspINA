project (COUNTING_REGION_PLUGIN)

include (GenerateExportHeader)

# Version Number
set (PLUGIN_VERSION_MAJOR 1)
set (PLUGIN_VERSION_MINOR 0)
set (PLUGIN_VERSION_PATCH 0)

cmake_minimum_required (VERSION 2.6)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

IF (NOT COMPILING_ESPINA)
  find_package(Espina REQUIRED)
  include(${ESPINA_USE_FILE})
ENDIF (NOT COMPILING_ESPINA)

include_directories(
  ${Espina_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # UI files
  )

qt4_add_resources (RCC_BUILT_SOURCES 
  rsc/CountingFrame.qrc
  )

# Create Interface files
qt4_wrap_ui (UI_BUILT_SOURCES
  CountingFramePanel.ui
  )

# We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
qt4_wrap_cpp(MOC_SRCS 
  CountingFramePanel.h
#   CountingFrameRenderer.h
#   Extensions/CountingFrameExtension.h
#   Extensions/StereologicalInclusion.h
#   CountingFrames/CountingFrame.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )


set (LOCAL_SOURCES
  CountingFramePanel.cpp
#   CountingFrameRenderer.cpp
#   Extensions/CountingFrameExtension.cpp
#   Extensions/StereologicalInclusion.cpp
  CountingFramePlugin.cpp
#   CountingFrames/vtkCountingFrameAxialSliceRepresentation.cpp
#   CountingFrames/vtkCountingFrameCoronalSliceRepresentation.cpp
#   CountingFrames/vtkCountingFrameSagittalSliceRepresentation.cpp
#   ColorEngines/CountingFrameColorEngine.cpp
#   CountingFrames/AdaptiveCountingFrame.cpp
#   CountingFrames/CountingFrame.cpp
#   CountingFrames/RectangularCountingFrame.cpp
#   CountingFrames/vtkCountingFrameSliceRepresentation.cpp
#   CountingFrames/vtkCountingFrameSliceWidget.cpp
#   CountingFrames/vtkCountingFrame3DRepresentation.cpp
#   CountingFrames/vtkCountingFrame3DWidget.cpp
  )

set (PLUGIN_SOURCES
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${MOC_SRCS}
  ${LOCAL_SOURCES}
  )

set (PLUGIN_LIBRARIES
  EspinaSupport
#   EspinaGUI
#   EspinaCore
#   EspinaUndo
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

target_link_libraries (CountingFramePlugin ${PLUGIN_LIBRARIES})

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set_target_properties(CountingFramePlugin PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

# install(
#   TARGETS CountingFrame
#   LIBRARY
#   DESTINATION bin/plugins
#   #COMPONENT CountingFrame
#   )
