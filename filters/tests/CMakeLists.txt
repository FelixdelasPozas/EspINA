SET(LIB_DEPENDECIES
  vtkRendering
  CajalFilters
)

SET (STACK_DIR "" CACHE PATH "Path to test Stack's folder")

add_subdirectory(ConnectedThresholdImageFilter)
add_subdirectory(ConnectedThresholdRegionGrowImageFilter)
