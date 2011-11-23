# Include libCajal header files
INCLUDE_DIRECTORIES (..)

ADD_DEFINITIONS(
    ${QT_DEFINITIONS}
)

SET(LIB_DEPENDECIES
    CajalData
    ${QT_LIBRARIES}
)

add_subdirectory(Taxonomy)