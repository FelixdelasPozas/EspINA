include_directories(.)

add_library(FilterSupport testing.cpp)

set(FILTERS_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${TESTING_DEPENDECIES}
  EspinaCoreTesting
  EspinaFiltersTesting
  EspinaTesting
  FilterSupport
)

add_subdirectory(VolumetricStreamReader)
add_subdirectory(SeedGrowSegmentation)
add_subdirectory(PlanarSplit)