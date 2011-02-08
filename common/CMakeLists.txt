#set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
#find_package(Cajal REQUIRED)

include_directories(
  .
  cache
  proxies
  ${CAJAL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES )
# Add extra library containing custom code for the client.
qt4_wrap_cpp(MOC_BUILT_SOURCES
selectionManager.h
objectManager.h
iPixelSelector.h
)

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)


file (GLOB LOCAL_SOURCES *.cpp)
file (GLOB CACHE_SOURCES cache/*.cpp )
file (GLOB PROXIES_SOURCES proxies/*.cpp )

set(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${CACHE_SOURCES}
  ${PROXIES_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )


add_library (Espina SHARED ${Espina_SOURCE_FILES})

target_link_libraries (Espina ${CAJAL_LIBRARIES})

add_subdirectory(tests)

#add_subdirectory(proxies)
