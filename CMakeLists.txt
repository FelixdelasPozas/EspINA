project (CAJAL)

cmake_minimum_required (VERSION 2.6)

# Version Number
set (CAJAL_VERSION_MAJOR 0)
set (CAJAL_VERSION_MINOR 3)

# SET (CTEST_INITIAL_CACHE  "
#   CXXFLAGS:STRING=-g -O0 -Wall -Wshadow -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage
#   CFLAGS:STRING=-g -O0 -Wall -W -fprofile-arcs -ftest-coverage
#   LDFLAGS:STRING=-fprofile-arcs -ftest-coverage
#   "
#   )

SET (STACK_DIR "" CACHE PATH "Path to test Stack's folder")

include (CTest)
ENABLE_TESTING()
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

SET (ITK_VTK_GLUE ${ITK_DIR}/../Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src)

#Has to be in the CMakeLists that declares a project
#may subprojects should be done and thus this move to
#the corresponding CMakeLists
include (CPack)

add_subdirectory (filters)
add_subdirectory (data)
add_subdirectory (tests)
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
