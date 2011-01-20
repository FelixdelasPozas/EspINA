# VTK Support
FIND_PACKAGE(VTK)
IF(VTK_FOUND)
        INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
        MESSAGE(FATAL_ERROR "VTK not found. Please set VTK_DIR.")
ENDIF(VTK_FOUND)

# ITK Support
FIND_PACKAGE(ITK)
IF(ITK_FOUND)
        INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
        MESSAGE(FATAL_ERROR "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

SET (ITK_VTK_GLUE "" CACHE PATH "Path to ItkVtkGlue external project")

INCLUDE_DIRECTORIES (
  ${ITK_VTK_GLUE}
#  ${ITK_DIR}/../Code/BasicFilters/
  )
#MESSAGE( STATUS "EEEE ${ITK_DIR}\n ${ITK_VTK_GLUE}\n   ${ITK_DIR}/../Code/BasicFilters/itkVTKImageExportBase.h" )

# 
FILE ( GLOB CORE_SOURCES *.cpp )

ADD_LIBRARY (CajalFilters ${CORE_SOURCES} )

# Maybe the target_link_libraries command is optional...
# (I think it depends if we use shared libraries)
TARGET_LINK_LIBRARIES (CajalFilters ITKCommon ITKBasicFilters ITKAlgorithms )

ADD_SUBDIRECTORY(tests)

