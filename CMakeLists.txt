project (CAJAL)

cmake_minimum_required (VERSION 2.6)

# Version Number
set (CAJAL_VERSION_MAJOR 0)
set (CAJAL_VERSION_MINOR 2)

#Has to be in the CMakeLists that declares a project
#may subprojects should be done and thus this move to
#the corresponding CMakeLists
ENABLE_TESTING()

add_subdirectory (filters)
add_subdirectory (data)
#add_subdirectory (gui)

get_target_property (CAJAL_DATA_LIBRARY CajalData LOCATION)
get_target_property (CAJAL_FILTERS_LIBRARY CajalFilters LOCATION)

set (CAJAL_INCLUDE ${PROJECT_SOURCE_DIR})

set (CAJAL_USE_FILE "${PROJECT_SOURCE_DIR}/CajalUse.cmake") 

configure_file (
  "${PROJECT_SOURCE_DIR}/CajalConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/CajalConfig.cmake"
  )

## TODO: To create installation .cmake, refer to Mastering CMake p.120
include (CPack)
