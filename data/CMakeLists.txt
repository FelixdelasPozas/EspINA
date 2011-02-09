#MESSAGE( STATUS "EEEE ${ITK_DIR}\n ${ITK_VTK_GLUE}\n   ${ITK_DIR}/../Code/BasicFilters/itkVTKImageExportBase.h" )

# 
file ( GLOB DATA_SOURCES *.cpp )

find_package (Qt4 REQUIRED)
if (QT4_FOUND)
	include (${QT_USE_FILE})
endif (QT4_FOUND)

add_library (CajalData ${DATA_SOURCES} )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
         SET_TARGET_PROPERTIES(CajalData PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )


# Maybe the target_link_libraries command is optional...
# (I think it depends if we use shared libraries)
#TARGET_LINK_LIBRARIES (Cajal ITKCommon ITKBasicFilters ITKAlgorithms )

add_subdirectory(tests)
