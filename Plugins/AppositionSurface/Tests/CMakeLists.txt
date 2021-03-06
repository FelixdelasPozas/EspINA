find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)
#include (${QT_USE_FILE})

find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})


INCLUDE_DIRECTORIES (
  ..
  ${QT_QTCORE_INCLUDE_DIR}
  )

SET(LIB_DEPENDECIES
  vtkRendering vtkInfovis
  ITKBasicFilters ITKIO ITKCommon 
  vtkHybrid vtkVolumeRendering
)

get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set (TEST_NAME AppositionPlane_test)

create_test_sourcelist(TESTS  
  ${TEST_NAME}.cpp
  Pipeline
  AppPlaneFeatures
  DecimalPlane
)

add_executable(${TEST_NAME} ${TESTS} 
  ../vtkAppositionPlaneFilter.cpp
  ../vtkAppositionPlaneFeatures.cpp
) 
target_link_libraries(${TEST_NAME} ${LIB_DEPENDECIES} ${QT_QTCORE_LIBRARY})

# Remove the test driver source file
set( TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ${TEST_NAME}.cpp)

# add tests
message(STATUS "STACKDIR " ${STACK_DIR})
FOREACH(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test("${COMPONENT_NAME}-${TName}" ${TEST_NAME} ${TName} ${STACK_DIR})
ENDFOREACH(TEST)
