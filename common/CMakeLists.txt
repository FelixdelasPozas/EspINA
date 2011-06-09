include_directories(
  cache
  proxies
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  ${ITK_DIR}/../
)

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES )
# Add extra library containing custom code for the client.
qt4_wrap_cpp(MOC_BUILT_SOURCES
  selectionManager.h
  taxonomyProxy.h
  sampleProxy.h
  espina.h
  pixelSelector.h
  selectionManager.h
  EspinaPlugin.h
  ui/volumeView.h
  ui/sliceView.h
  volumetricRenderer.h
  meshRenderer.h
  EspinaPlugin.h
  labelMapExtension.h
  crosshairExtension.h
)

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)

file (GLOB LOCAL_SOURCES *.cpp)
file (GLOB CACHE_SOURCES cache/*.cpp )
file (GLOB PROXIES_SOURCES proxies/*.cpp )
file (GLOB UI_SOURCES ui/*.cpp )

set(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${CACHE_SOURCES}
  ${PROXIES_SOURCES}
  ${UI_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  )

set(EXTERNAL_LIBS zip)

add_library (Espina SHARED ${Espina_SOURCE_FILES})

target_link_libraries (Espina ${CAJAL_LIBRARIES} ${EXTERNAL_LIBS})

MESSAGE (STATUS "Paraview Libraries:" ${PARAVIEW_LIBRARIES})

install (TARGETS Espina ${PARAVIEW_LIBRARIES} LIBRARY DESTINATION lib)

add_subdirectory(tests)

#add_subdirectory(proxies)
