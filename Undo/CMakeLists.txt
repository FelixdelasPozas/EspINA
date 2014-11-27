include_directories(
  ${CMAKE_BINARY_DIR}         # Exports & Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

set (Undo_SOURCES
  ${UNDO_MOC}
  ${UNDO_UI}
  AddCategoryCommand.cpp
  AddClassificationCommand.cpp
  AddSegmentations.cpp
  AddRelationCommand.cpp
  ChangeCategoryCommand.cpp
  ChangeSegmentationNotes.cpp
  ChangeSegmentationTags.cpp
  ModifyDataCommand.cpp
  RemoveCategoryCommand.cpp
  RemoveSegmentations.cpp
  ReparentCategoryCommand.cpp
  ReplaceOutputCommand.cpp
  )

set(Undo_EXTERNAL_LIBS
  EspinaCore
  EspinaSupport
  EspinaGUI
)

if(DEFINED MINGW)
  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    SET(Undo_SOURCES ${Undo_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    ENABLE_LANGUAGE(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW)

add_library (EspinaUndo SHARED ${Undo_SOURCES})

target_link_libraries (EspinaUndo ${Undo_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
             EspinaUndo
             BASE_NAME EspinaUndo
             EXPORT_MACRO_NAME EspinaUndo_EXPORT
             EXPORT_FILE_NAME EspinaUndo_Export.h
  )
 
install(TARGETS EspinaUndo
  # IMPORTANT: Add the EspinaUndo library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

