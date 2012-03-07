add_subdirectory(proxies)

set (MOC_SOURCES
  ${MOC_SOURCES}
  model/EspinaModel.h
  model/ModelTest.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  model/EspinaFactory.cpp
  model/EspinaModel.cpp
  model/ModelItem.cpp
  model/ModelTest.cpp
  model/Segmentation.cpp
  model/Channel.cpp
  model/RelationshipGraph.cpp
  model/Sample.cpp
  model/Taxonomy.cpp
  PARENT_SCOPE
  )

# if (${BUILD_TESTING})
  add_subdirectory(tests)
# endif (${BUILD_TESTING})

add_library(model_fage
  EspinaFactory.cpp
  EspinaModel.cpp
  ModelItem.cpp
  ModelTest.cpp
  Segmentation.cpp
  Channel.cpp
  RelationshipGraph.cpp
  Sample.cpp
  Taxonomy.cpp
  )