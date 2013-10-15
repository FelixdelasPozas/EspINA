set (MOC_SOURCES
  ${MOC_SOURCES}
  model/proxies/SampleProxy.h
  model/proxies/TaxonomyProxy.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  model/proxies/SampleProxy.cpp
  model/proxies/TaxonomyProxy.cpp
  PARENT_SCOPE
  )

if (${USE_KDEVELOP_FAKE_TARGETS})
add_library(proxies_fake
  SampleProxy.cpp
  TaxonomyProxy.cpp
  )
endif (${USE_KDEVELOP_FAKE_TARGETS})