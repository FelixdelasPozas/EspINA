find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
#   ${ITK_DIR}/../
)

# find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
# link_directories ( ${Boost_LIBRARY_DIRS} )
#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES
  )

# Add extra library containing custom code for the client.
qt4_wrap_cpp(MOC_BUILT_SOURCES
  model/EspINA.h
  model/ModelTest.h
  gui/ActionSelectorWidget.h
  gui/EspinaView.h
  gui/SliceView.h
  gui/ViewManager.h
  gui/VolumeView.h
  selection/SelectionHandler.h
  selection/SelectionManager.h
  )

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)

set(LOCAL_SOURCES
  )

set(CACHE_SOURCES
  cache/Cache.cpp
  cache/CachedObjectBuilder.cpp
  cache/CacheHash.cpp
  )

set(GUI_SOURCES
  gui/ActionSelectorWidget.cpp
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/EspinaView.cpp
  gui/SliceView.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
  )

set(MODEL_SOURCES
  model/EspINA.cpp
  model/ModelTest.cpp
  model/Segmentation.cpp
  model/Channel.cpp
  model/Sample.cpp
  model/Taxonomy.cpp
  )

set(PROCESSING_SOURCES
  processing/pqFilter.cpp
  processing/pqData.cpp
  )

set(SELECTION_SOURCES
  selection/PixelSelector.cpp
  selection/SelectionHandler.cpp
  selection/SelectionManager.cpp
  )

set(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${CACHE_SOURCES}
  ${GUI_SOURCES}
  ${MODEL_SOURCES}
  ${PROCESSING_SOURCES}
  ${SELECTION_SOURCES}
#   ${PROXIES_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set(EXTERNAL_LIBS
  ${QUAZIP_LIBRARIES}
  QtTest
#   ${Boost_LIBRARIES}
)

add_library (Espina SHARED ${Espina_SOURCE_FILES})

target_link_libraries (Espina ${EXTERNAL_LIBS})

install (
  TARGETS Espina ${PARAVIEW_LIBRARIES}
  LIBRARY
  DESTINATION lib
  COMPONENT espina
  )

if (${BUILD_TESTING})
  add_subdirectory(model/tests)
endif (${BUILD_TESTING})