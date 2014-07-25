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
  AddRelationCommand.cpp
  ChangeCategoryCommand.cpp
  ChangeSegmentationNotes.cpp
  ChangeSegmentationTags.cpp
  RemoveCategoryCommand.cpp
  RemoveSegmentations.cpp
  ReparentCategoryCommand.cpp
  ReplaceOutputCommand.cpp
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

target_link_libraries (EspinaUndo ${UNDO_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
             EspinaUndo
             BASE_NAME EspinaUndo
             EXPORT_MACRO_NAME EspinaUndo_EXPORT
             EXPORT_FILE_NAME EspinaUndo_Export.h
  )

#set (UNDO_HEADERS 
#  #TODO: set all header files required to develop external apps
#  "${CMAKE_CURRENT_BINARY_DIR}/EspinaUndo_Export.h"
#  )
#
#set_target_properties(EspinaUndo PROPERTIES
#  PUBLIC_HEADER "${UNDO_HEADERS}")
 
install(TARGETS EspinaUndo
  # IMPORTANT: Add the EspinaUndo library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )
