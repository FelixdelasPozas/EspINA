set (MOC_SOURCES
  ${MOC_SOURCES}
  model/proxies/SampleProxy.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  model/proxies/SampleProxy.cpp
  PARENT_SCOPE
  )

# add_library(fake SampleProxy.cpp)