enable_testing()

set (BUILD_UNIT_TESTS OFF  CACHE BOOL "Build ESPINA Unit Tests")

set (BUILD_CI_TESTS   OFF  CACHE BOOL "Build ESPINA Continous Integration Tests")

set (USE_DASHBOARD    OFF  CACHE BOOL "Use ESPINA Unitary Tests Dashboard")

if (${USE_DASHBOARD})

 set (BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  mark_as_advanced (BUILDNAME)

endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  .
  )

set( CORE_DIR       ${CMAKE_SOURCE_DIR}/Core       )
set( GUI_DIR        ${CMAKE_SOURCE_DIR}/GUI        )
set( EXTENSIONS_DIR ${CMAKE_SOURCE_DIR}/Extensions )
set( FILTERS_DIR    ${CMAKE_SOURCE_DIR}/Filters    )
set( APP_DIR        ${CMAKE_SOURCE_DIR}/App        )

set (TESTING_SOURCES
  testing_support_channel_input.cpp
  testing_support_dummy_filter.cpp
  Testing_Support.cpp
)

add_library(EspinaTesting SHARED ${TESTING_SOURCES})

set(TESTING_DEPENDECIES
  EspinaTesting
  )

if (BUILD_UNIT_TESTS)

  add_subdirectory(Unitary)

endif(BUILD_UNIT_TESTS)

if (BUILD_CI_TESTS)

  add_subdirectory(Integration)

endif(BUILD_CI_TESTS)
