find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
link_directories ( ${Boost_LIBRARY_DIRS} )

set(LIB_DEPENDECIES
 EspinaCore
 ${QT_LIBRARIES}
 ${Boost_LIBRARIES}
)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/common
  ${CMAKE_SOURCE_DIR}/common/IO
)

add_subdirectory(IO)
