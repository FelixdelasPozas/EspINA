project (COUNTING_REGION_PLUGIN)

# Version Number
set (PLUGIN_VERSION_MAJOR 0)
set (PLUGIN_VERSION_MINOR 2)
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
  CountingFrameRenderer.h
  Extensions/CountingFrameExtension.h
  Extensions/StereologicalInclusion.h
  CountingFrames/CountingFrame.h

  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )


# file ( GLOB LOCAL_SOURCES *.cpp )
set (LOCAL_SOURCES
  CountingFramePanel.cpp
  CountingFrameRenderer.cpp
  Extensions/CountingFrameExtension.cpp
  Extensions/StereologicalInclusion.cpp
  )

set (REGION_SOURCES
  CountingFrames/vtkCountingFrameAxialSliceRepresentation.cpp
  CountingFrames/vtkCountingFrameCoronalSliceRepresentation.cpp
  CountingFrames/vtkCountingFrameSagittalSliceRepresentation.cpp
  ColorEngines/CountingFrameColorEngine.cpp
  CountingFrames/AdaptiveCountingFrame.cpp
  CountingFrames/CountingFrame.cpp
  CountingFrames/RectangularCountingFrame.cpp
  CountingFrames/vtkCountingFrameSliceRepresentation.cpp
  CountingFrames/vtkCountingFrameSliceWidget.cpp
  CountingFrames/vtkCountingFrame3DRepresentation.cpp
  CountingFrames/vtkCountingFrame3DWidget.cpp
  )

set (PLUGIN_SOURCES
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${MOC_SRCS}
  ${LOCAL_SOURCES}
  ${REGION_SOURCES}
  )

set (PLUGIN_LIBRARIES
  EspinaCore
  EspinaUndo
  )

add_library(CountingFramePlugin SHARED ${PLUGIN_SOURCES})
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
