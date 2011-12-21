find_package(Quazip REQUIRED)
include(${QUAZIP_USE_FILE})

include_directories(
  cache
  proxies
  gui
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
  gui/SliceView.h
  gui/ViewManager.h
  )

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)

set (LOCAL_SOURCES
  )
set (MODEL_SOURCES
  model/EspINA.cpp
  model/ModelTest.cpp
  )

set (GUI_SOURCES
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/SliceView.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  )

set(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${MODEL_SOURCES}
#   ${CACHE_SOURCES}
#   ${PROXIES_SOURCES}
  ${GUI_SOURCES}
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

install (TARGETS Espina ${PARAVIEW_LIBRARIES} LIBRARY DESTINATION lib)

# add_subdirectory(tests)