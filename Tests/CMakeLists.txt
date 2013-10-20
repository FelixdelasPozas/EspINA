enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build EspINA Unit Tests")

set (BUILD_CI_TESTS   OFF  CACHE BOOL "Build EspINA Continous Integration Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use EspINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()



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

