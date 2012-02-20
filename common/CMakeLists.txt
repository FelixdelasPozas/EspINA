include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
#   ${ITK_DIR}/../
)

find_package(Boost REQUIRED COMPONENTS  graph) #system filesystem 
link_directories ( ${Boost_LIBRARY_DIRS} )

#------------------------------------------------------------------------------
# Add Qt Resource files
qt4_add_resources(RCC_BUILT_SOURCES
  )

# Add extra library containing custom code for the client.
qt4_wrap_cpp(MOC_BUILT_SOURCES
  model/EspinaModel.h
  model/ModelTest.h
  gui/ActionSelectorWidget.h
  gui/ActionSelector.h
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
  EspinaCore.cpp
  )

set(CACHE_SOURCES
  cache/Cache.cpp
  cache/CachedObjectBuilder.cpp
  cache/CacheHash.cpp
  )

set(GUI_SOURCES
  gui/ActionSelectorWidget.cpp
  gui/ActionSelector.cpp
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/EspinaView.cpp
  gui/SliceView.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
  )

set(MODEL_SOURCES
  model/EspinaFactory.cpp
  model/EspinaModel.cpp
  model/ModelItem.cpp
  model/ModelTest.cpp
  model/Segmentation.cpp
  model/Channel.cpp
  model/RelationshipGraph.cpp
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

set (UNDO_SOURCES
  undo/AddChannel.cpp
  undo/AddFilter.cpp
  undo/AddRelation.cpp
  undo/AddSample.cpp
  undo/AddSegmentation.cpp
  )

set(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${CACHE_SOURCES}
  ${GUI_SOURCES}
  ${MODEL_SOURCES}
  ${PROCESSING_SOURCES}
  ${SELECTION_SOURCES}
  ${UNDO_SOURCES}
#   ${PROXIES_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )

set(EXTERNAL_LIBS
  ${Boost_LIBRARIES}
  QtTest
)

add_library (EspinaCore SHARED ${Espina_SOURCE_FILES})

target_link_libraries (EspinaCore ${EXTERNAL_LIBS})

install (
  TARGETS EspinaCore ${PARAVIEW_LIBRARIES}
  LIBRARY
  DESTINATION lib
  COMPONENT espina
  )

add_subdirectory (plugins/EspinaWidgets)
if (${BUILD_TESTING})
  add_subdirectory(model/tests)
endif (${BUILD_TESTING})