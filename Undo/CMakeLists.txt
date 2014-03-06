include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

set (UNDO_SOURCES
  ${UNDO_MOC}
  ${UNDO_UI}
  AddCategoryCommand.cpp
  AddClassificationCommand.cpp
  AddSegmentations.cpp
  ChangeCategoryCommand.cpp
  RemoveCategoryCommand.cpp
  RemoveSegmentations.cpp
  ReparentCategoryCommand.cpp
  )

set(UNDO_EXTERNAL_LIBS
  EspinaCore
  EspinaGUI
)

if(DEFINED MINGW OR DEFINED MSVC)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(UNDO_SOURCES ${UNDO_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW OR DEFINED MSVC)

add_library (EspinaUndo SHARED ${UNDO_SOURCES})
GENERATE_EXPORT_HEADER(
             EspinaUndo
             BASE_NAME EspinaUndo
             EXPORT_MACRO_NAME EspinaUndo_EXPORT
             EXPORT_FILE_NAME EspinaUndo_Export.h
  )
target_link_libraries (EspinaUndo ${UNDO_EXTERNAL_LIBS})
