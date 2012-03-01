find_package (ParaView REQUIRED)
include (${PARAVIEW_USE_FILE})

set (MOC_SOURCES
  ${MOC_SOURCES}
  views/pqSliceView.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  views/pqSliceView.cpp
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  views/vtkPVSliceView.cpp
  views/vtkSMSliceViewProxy.cpp
  views/vtkChannelRepresentation.cpp
  views/vtkSegmentationRepresentation.cpp
  views/vtkSliceRepresentation.cpp
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  views/EspinaViews.xml
  PARENT_SCOPE
  )