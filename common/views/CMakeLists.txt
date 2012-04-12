set (MOC_SOURCES
  ${MOC_SOURCES}
  views/pqSliceView.h
  views/pqVolumeView.h
  PARENT_SCOPE
  )

set (CORE_SOURCES
  ${CORE_SOURCES}
  views/pqSliceView.cpp
  views/pqVolumeView.cpp
  PARENT_SCOPE
  )

set (PVCORE_SOURCES
  ${PVCORE_SOURCES}
  views/vtkPVSliceView.cpp
  views/vtkPVVolumeView.cpp
  views/vtkSMSliceViewProxy.cpp
  views/vtkChannelRepresentation.cpp
  views/vtkSegmentationRepresentation.cpp
  views/vtkSliceRepresentation.cpp
  views/vtkVolumetricRepresentation.cpp
  PARENT_SCOPE
  )

set (PVCORE_XML
  ${PVCORE_XML}
  views/EspinaViews.xml
  PARENT_SCOPE
  )

add_library(fake
  pqSliceView.cpp
  pqVolumeView.cpp
  vtkPVSliceView.cpp
  vtkPVVolumeView.cpp
  vtkSMSliceViewProxy.cpp
  vtkChannelRepresentation.cpp
  vtkSegmentationRepresentation.cpp
  vtkSliceRepresentation.cpp
  vtkVolumetricRepresentation.cpp
)
