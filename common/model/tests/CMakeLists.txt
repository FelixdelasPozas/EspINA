# Include libCajal header files
INCLUDE_DIRECTORIES (..)

ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
)

SET(LIB_DEPENDECIES
  Espina
  EspinaViews
  ${QT_LIBRARIES}
)

add_subdirectory(Taxonomy)