enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build EspINA Unit Tests")

set (BUILD_CI_TESTS   OFF  CACHE BOOL "Build EspINA Continous Integration Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use EspINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  )

set( CORE_DIR       ${CMAKE_SOURCE_DIR}/Core       )
set( GUI_DIR        ${CMAKE_SOURCE_DIR}/GUI        )
set( EXTENSIONS_DIR ${CMAKE_SOURCE_DIR}/Extensions )
set( FILTERS_DIR    ${CMAKE_SOURCE_DIR}/Filters    )
set( APP_DIR        ${CMAKE_SOURCE_DIR}/App        )


if (BUILD_UNIT_TESTS)

  add_subdirectory(Unitary)

endif(BUILD_UNIT_TESTS)

if (BUILD_CI_TESTS)

  add_subdirectory(Integration)

endif(BUILD_CI_TESTS)
