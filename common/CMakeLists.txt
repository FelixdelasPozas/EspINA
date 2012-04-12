find_package (VTK REQUIRED)
include (${VTK_USE_FILE})

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})
#NOTE: We can't use ITK and BOOST graph at the same time because of
#      ITK_REQUIRED_CXX_FLAG: -ftemplate-depth-50
#      so we override CXX_FLAGS to unset previous flag
set(CMAKE_CXX_FLAGS " -Wall -Wno-deprecated -msse2")

find_package(Boost REQUIRED COMPONENTS  graph) #system filesystem
link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/common/extensions/Margins
  ${CMAKE_SOURCE_DIR}/common/extensions/Morphological
  ${CMAKE_SOURCE_DIR}/common/plugins/EspinaWidgets
  ${CMAKE_SOURCE_DIR}/common/views
  ${CMAKE_CURRENT_BINARY_DIR}  #For wrap/ui files
  ${CMAKE_CURRENT_BINARY_DIR}/plugins/EspinaWidgets
  ${ITK_VTK_GLUE}
)

set (MOC_SOURCES
  EspinaCore.h
  settings/GeneralSettings.h
  )

set (CORE_SOURCES
  EspinaCore.cpp
  File.cpp
  settings/GeneralSettings.cpp
  )

set (PVCORE_SOURCES)


set (PVCORE_XML)

add_subdirectory(cache)
add_subdirectory(extensions)
add_subdirectory(gui)
add_subdirectory(model)
add_subdirectory(plugins)
add_subdirectory(processing)
add_subdirectory(selection)
add_subdirectory(undo)
add_subdirectory(views)

# Add extra library containing custom code for the client.
qt4_wrap_cpp(CORE_MOC
  ${MOC_SOURCES}
  )

# if no target is processed auto-generated files are not created
# so we need to call cmake macros here
ADD_3DWIDGET(
  WIDGET_IFACES WIDGET_SOURCES
  CLASS_NAME pqNonRotatingBoxWidget
  WIDGET_TYPE NonRotatingBox
  )

ADD_PARAVIEW_VIEW_MODULE(
  VIEW_IFACES VIEW_SOURCES
  VIEW_TYPE pqSliceView
  VIEW_XML_GROUP views
  )

ADD_PARAVIEW_VIEW_MODULE(
  VOLUMEVIEW_IFACES VOLUMEVIEW_SOURCES
  VIEW_TYPE pqVolumeView
  VIEW_XML_GROUP views
  )

set (PVCORE_IFACES
  ${WIDGET_IFACES}
  ${VIEW_IFACES}
  ${VOLUMEVIEW_IFACES}
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  ${CORE_MOC}
  ${WIDGET_SOURCES}
  ${VIEW_SOURCES}
  ${VOLUMEVIEW_SOURCES}
  )

# Make debug output clearer
foreach(arg ${CORE_SOURCES})
   set(MSG_SOURCES "${MSG_SOURCES}\n *${arg}")
 endforeach(arg ${CORE_SOURCES})
message("SOURCES:${MSG_SOURCES}")

foreach(arg ${PVCORE_SOURCES})
   set(MSG_PVSOURCES "${MSG_PVSOURCES}\n *${arg}")
 endforeach(arg ${PVCORE_SOURCES})
message("PV SOURCES:${MSG_PVSOURCES}")

foreach(arg ${PVCORE_XML})
   set(MSG_PVXML "${MSG_PVXML}\n *${arg}")
 endforeach(arg ${PVCORE_XML})
message("PV XML:${MSG_PVXML}")


set(EXTERNAL_LIBS
  ${Boost_LIBRARIES}
  QtTest
)

add_library (EspinaCore SHARED ${CORE_SOURCES})
target_link_libraries (EspinaCore ${EXTERNAL_LIBS})

install (
  TARGETS EspinaCore
  LIBRARY
  DESTINATION lib
  COMPONENT espina
  )

ADD_PARAVIEW_PLUGIN(
  EspinaPVCore
  "${ESPINA_VERSION_MAJOR}.${ESPINA_VERSION_MINOR}"
  SERVER_MANAGER_XML ${PVCORE_XML}
  SERVER_MANAGER_SOURCES ${PVCORE_SOURCES}
#   SOURCES ${PLUGIN_SOURCES}
  GUI_INTERFACES ${PVCORE_IFACES}
  )

set (PVCORE_LIBRARIES
  ITKCommon ITKBasicFilters ITKAlgorithms
  )

target_link_libraries(EspinaPVCore ${PVCORE_LIBRARIES})