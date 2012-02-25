set (MOC_SOURCES
  ${MOC_SOURCES}
  gui/ActionSelector.h
  gui/ActionSelectorWidget.h
  gui/EspinaView.h
  gui/SliceView.h
  gui/ViewManager.h
  gui/VolumeView.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  gui/ActionSelector.cpp
  gui/ActionSelectorWidget.cpp
  gui/DynamicWidget.cpp
  gui/EspinaDockWidget.cpp
  gui/EspinaView.cpp
  gui/SliceView.cpp
  gui/ViewFrame.cpp
  gui/ViewManager.cpp
  gui/VolumeView.cpp
  PARENT_SCOPE
  )