include (GenerateExportHeader)

find_package(XLSLIB REQUIRED)

find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
  ${XLSLIB_INCLUDE_DIRS}
)

qt4_wrap_cpp(Support_MOC
  Plugin.h
  Representations/RepresentationSwitch.h
  Representations/Slice3DManager.h
  Widgets/Panel.h
  Widgets/TabularReport.h
  Widgets/TabularReportEntry.h
  Widgets/ProgressTool.h
  Widgets/EditTool.h
  Widgets/PanelSwitch.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

qt4_wrap_ui (Support_UI
  Widgets/TabularReportEntry.ui
  )

if (METADONA_FOUND)
  qt4_wrap_cpp(METADONA_MOC
    Metadona/EntrySelectorDialog.h
    Metadona/EntryEditorDialog.h
    Metadona/LevelSelectorDialog.h
    Metadona/MetadataViewer.h
    )
  qt4_wrap_ui (METADONA_UI
    Metadona/EntrySelectorDialog.ui
    Metadona/EntryEditorDialog.ui
    Metadona/LevelSelectorDialog.ui
    Metadona/MetadataViewer.ui
    )
  set (METADONA_SOURCES
    Metadona/Coordinator.cpp
    Metadona/EntrySelectorDialog.cpp
    Metadona/EntryEditorDialog.cpp
    Metadona/LevelSelectorDialog.cpp
    Metadona/MetadataViewer.cpp
    Metadona/EntryWidget.cpp
    Metadona/StorageFactory.cpp
    )
endif (METADONA_FOUND)


set (Support_SOURCES
  ${Support_MOC}
  ${Support_UI}
  ${METADONA_MOC}
  ${METADONA_UI}
  ${METADONA_SOURCES}
  Factory/FilterRefinerFactory.cpp
  Readers/ChannelReader.cpp
  Representations/BasicRepresentationSwitch.cpp
  Representations/RepresentationSwitch.cpp
  Representations/Slice3DManager.cpp
  Representations/SliceManager.cpp
  ROIAccumulator.cpp
  Settings/Settings.cpp
  Settings/SettingsPanel.cpp
  Utils/FactoryUtils.cpp
  Utils/xlsUtils.cpp
  Context.cpp
  Report.cpp
  ContextFactories.cpp
  Widgets/ActionGroupWidget.cpp
  Widgets/Panel.cpp
  Widgets/TabularReport.cpp
  Widgets/TabularReportEntry.cpp
  Widgets/ProgressTool.cpp
  Widgets/PanelSwitch.cpp
  Widgets/ColorEngineSwitch.cpp
  Widgets/EditTool.cpp
  )

set(Support_EXTERNAL_LIBS
  EspinaCore
  EspinaGUI
  EspinaFilters
  ${METADONA_LIBRARIES}
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${XLSLIB_LIBRARIES}
)

if(DEFINED MINGW)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(Support_SOURCES ${Support_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW)

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
if(DEFINED MINGW OR DEFINED MSVC)
  SET(Support_EXTERNAL_LIBS ${Support_EXTERNAL_LIBS} iconv)
endif(DEFINED MINGW OR DEFINED MSVC)
target_link_libraries (EspinaSupport ${Support_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
             EspinaSupport
             BASE_NAME EspinaSupport
             EXPORT_MACRO_NAME EspinaSupport_EXPORT
             EXPORT_FILE_NAME EspinaSupport_Export.h
  )

install(TARGETS EspinaSupport
  # IMPORTANT: Add the EspinaSupport library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")