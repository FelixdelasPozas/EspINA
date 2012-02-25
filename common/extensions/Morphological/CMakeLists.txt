set (CORE_SOURCES
  ${CORE_SOURCES}
  extensions/Morphological/MorphologicalExtension.cpp
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  extensions/Morphological/vtkMorphologicalFeaturesFilter.cpp
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  extensions/Morphological/MorphologicalExtension.xml
  PARENT_SCOPE
  )