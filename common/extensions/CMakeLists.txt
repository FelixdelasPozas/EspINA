add_subdirectory(Margins)
add_subdirectory(Morphological)

set (CORE_SOURCES
  ${CORE_SOURCES}
  extensions/ModelItemExtension.cpp
  extensions/ChannelExtension.cpp
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

add_library(fake_extensions
  ModelItemExtension.cpp
  ChannelExtension.cpp
  SegmentationExtension.cpp
  )