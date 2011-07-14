# Find required libraries
## Qt Support
find_package (Qt4 REQUIRED)
if (QT4_FOUND)
	include (${QT_USE_FILE})
endif (QT4_FOUND)


file ( GLOB TESTS_SOURCES *.cpp )

add_library (CajalTests ${TESTS_SOURCES} )

if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
         SET_TARGET_PROPERTIES(CajalTests PROPERTIES COMPILE_FLAGS "-fPIC")
endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

set_target_properties(CajalTests PROPERTIES VERSION "${CAJAL_VERSION_MAJOR}.${CAJAL_VERSION_MINOR}")

# Maybe the target_link_libraries command is optional...
# (I think it depends if we use shared libraries)
#TARGET_LINK_LIBRARIES (Cajal ITKCommon ITKBasicFilters ITKAlgorithms )
