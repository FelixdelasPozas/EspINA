# Include libCajal header files
INCLUDE_DIRECTORIES (..)

ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
)
find_package (ParaView REQUIRED 1.12) # Includes Qt4
include (${PARAVIEW_USE_FILE})

SET(LIB_DEPENDECIES
  EspinaCore
  EspinaPVCore
  pqApplicationComponents
#   pqComponents
#   pqCore
  ${QT_LIBRARIES}
)

add_subdirectory(Taxonomy)