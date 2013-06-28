SET(HELPER_SOURCES
  ${CMAKE_SOURCE_DIR}/App/Undo/AtomicModelOperations.cpp
  ${CMAKE_SOURCE_DIR}/Undo/ChangeTaxonomyCommand.cpp
  )
SET(LIB_DEPENDECIES
  vtkRendering
  vtkIO
)

add_subdirectory( Core )
