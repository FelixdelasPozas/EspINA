#MESSAGE( STATUS "EEEE ${ITK_DIR}\n ${ITK_VTK_GLUE}\n   ${ITK_DIR}/../Code/BasicFilters/itkVTKImageExportBase.h" )

# 
file ( GLOB DATA_SOURCES *.cpp )

add_library (CajalData ${DATA_SOURCES} )

# Maybe the target_link_libraries command is optional...
# (I think it depends if we use shared libraries)
#TARGET_LINK_LIBRARIES (Cajal ITKCommon ITKBasicFilters ITKAlgorithms )

add_subdirectory(tests)

