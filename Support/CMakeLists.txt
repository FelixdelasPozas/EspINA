include (GenerateExportHeader)

find_package(XLSLIB REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
  ${XLSLIB_INCLUDE_DIRS}
)

qt4_wrap_cpp(Support_MOC
  DockWidget.h
  EventHandler.h
  Plugin.h
  ToolGroup.h
  ViewManager.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

# qt4_wrap_ui (Support_UI
#   )

set (Support_SOURCES
  ${Support_MOC}
  ${Support_UI}
  ActionGroupWidget.cpp
  EventHandler.cpp
  Factory/DefaultChannelExtensionFactory.cpp
  Factory/DefaultSegmentationExtensionFactory.cpp
  ToolGroup.cpp
  Utils/FactoryUtils.cpp
  Utils/xlsUtils.cpp
  ViewManager.cpp
  )

set(Support_EXTERNAL_LIBS
  EspinaCore
  EspinaGUI
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${XLSLIB_LIBRARIES}
)

if(DEFINED MINGW OR DEFINED MSVC)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(Support_SOURCES ${Support_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW OR DEFINED MSVC)

if(APPLE)
   include_directories ( /System/Library/Frameworks )
   find_library(COCOA_LIBRARY Cocoa)
   find_library(GLUT_LIBRARY GLUT )
   find_library(OpenGL_LIBRARY OpenGL )
   mark_as_advanced (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   set(Support_EXTERNAL_LIBS ${Support_EXTERNAL_LIBS} ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
endif (APPLE)

add_library (EspinaSupport SHARED ${Support_SOURCES})
GENERATE_EXPORT_HEADER(
             EspinaSupport
             BASE_NAME EspinaSupport
             EXPORT_MACRO_NAME EspinaSupport_EXPORT
             EXPORT_FILE_NAME EspinaSupport_Export.h
  )

target_link_libraries (EspinaSupport ${Support_EXTERNAL_LIBS})