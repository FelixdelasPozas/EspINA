include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

set (FILTERS_SOURCES
  ${FILTERS_MOC}
  ${FILTERS_UI}
  ChannelReader.cpp
  ClosingFilter.cpp
  ContourSource.cpp
  DilateFilter.cpp
  ErodeFilter.cpp
  FillHolesFilter.cpp
  FreeFormSource.cpp
  ImageLogicFilter.cpp
  MorphologicalEditionFilter.cpp
  OpeningFilter.cpp
  SeedGrowSegmentationFilter.cpp
  SplitFilter.cpp
  )

set(FILTERS_EXTERNAL_LIBS
  EspinaCore
)

add_library          ( EspinaFilters SHARED ${FILTERS_SOURCES})
target_link_libraries( EspinaFilters ${FILTERS_EXTERNAL_LIBS} )