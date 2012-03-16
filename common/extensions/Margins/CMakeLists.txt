set (CORE_SOURCES
  ${CORE_SOURCES}
  extensions/Margins/MarginsChannelExtension.cpp
  extensions/Margins/MarginsSegmentationExtension.cpp
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  extensions/Margins/vtkChannelBorderDetector.cpp
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  extensions/Margins/Margins.xml
  PARENT_SCOPE
  )

add_library(fake_margindetector
  MarginsChannelExtension.cpp
  MarginsSegmentationExtension.cpp
  vtkChannelBorderDetector.cpp
  )