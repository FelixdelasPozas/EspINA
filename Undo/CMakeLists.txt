include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

set (UNDO_SOURCES
  ${UNDO_MOC}
  ${UNDO_UI}
  AddChannel.cpp
  AddFilter.cpp
  AddRelation.cpp
  AddSample.cpp
  AddSegmentation.cpp
  ChangeTaxonomyCommand.cpp
  TaxonomiesCommand.cpp
  ChangeSegmentationNotes.cpp
  ChangeSegmentationTags.cpp
  FillHolesCommand.cpp
  RemoveSegmentation.cpp
  )

set(UNDO_EXTERNAL_LIBS
  EspinaCore
)

add_library (EspinaUndo SHARED ${UNDO_SOURCES})
target_link_libraries (EspinaUndo ${UNDO_EXTERNAL_LIBS})
