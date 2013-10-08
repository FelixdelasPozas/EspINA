enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build EspINA Unit Tests")

set (BUILD_CI_TESTS   OFF  CACHE BOOL "Build EspINA Continous Integration Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use EspINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()


# Generate coverage information
set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")

set( CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

set( CORE_DIR ${CMAKE_SOURCE_DIR}/Core )


if (BUILD_UNIT_TESTS)

  add_subdirectory(Unitary)

endif(BUILD_UNIT_TESTS)

#SET(HELPER_SOURCES
#  ${CMAKE_SOURCE_DIR}/App/Undo/AtomicModelOperations.cpp
#  ${CMAKE_SOURCE_DIR}/Undo/ChangeTaxonomyCommand.cpp
#  )
#SET(LIB_DEPENDECIES
#  vtkRendering
#  vtkIO
#)

