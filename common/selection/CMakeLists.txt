set (MOC_SOURCES
  ${MOC_SOURCES}
  selection/SelectionHandler.h
  selection/SelectionManager.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  selection/PixelSelector.cpp
  selection/SelectionHandler.cpp
  selection/SelectionManager.cpp
  PARENT_SCOPE
  )