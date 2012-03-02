add_subdirectory(Morphological)

set (CORE_SOURCES
  ${CORE_SOURCES}
  extensions/SegmentationExtension.cpp
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  PARENT_SCOPE
  )