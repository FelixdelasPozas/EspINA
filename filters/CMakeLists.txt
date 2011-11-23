# Find required libraries

# VTK Support
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# ITK Support
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set (ITK_VTK_GLUE ${ITK_DIR}/../Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src)

include_directories (
	${ITK_VTK_GLUE}
        ${ITK_DIR}/../Code/Review/
	)

file ( GLOB FILTER_SOURCES *.cpp )

add_library (CajalFilters ${FILTER_SOURCES})

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
          set_target_properties(CajalFilters PROPERTIES COMPILE_FLAGS "-fPIC")
endif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")


set_target_properties(CajalFilters PROPERTIES VERSION 0.1)

# Maybe the target_link_libraries command is optional...
# (I think it depends if we use shared libraries)
target_link_libraries (CajalFilters ITKCommon ITKBasicFilters ITKAlgorithms ITKIO vtkIO)

SET (ENABLE_TESTS 1)

if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)


# INSTALL
install (TARGETS CajalFilters
	ARCHIVE DESTINATION /lib
	)
