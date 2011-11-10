find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

INCLUDE_DIRECTORIES (
  ../..
  ${QT_QTCORE_INCLUDE_DIR}
  )

# get_filename_component( COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set (COMPONENT_NAME Taxonomy)

set (TEST_NAME ${COMPONENT_NAME}_test)

create_test_sourcelist(TESTS
   ${TEST_NAME}.cpp
   CreateTaxonomy
   TaxonomyProperties
)
#   insertInANonExistentSupElement.cpp  
#   repeatedElement.cpp
#   writing_reading.cpp

add_executable(${TEST_NAME} ${TESTS})
target_link_libraries(${TEST_NAME} ${LIB_DEPENDECIES} ${QT_QTCORE_LIBRARY}  CajalTests)

# Remove the test driver source file
set  (TestToRun ${TESTS} )
list (REMOVE_ITEM TestToRun ${TEST_NAME}.cpp)

# add tests
FOREACH(TEST ${TestToRun})
  get_filename_component(TName ${TEST} NAME_WE)
  add_test("${COMPONENT_NAME}-${TName}" ${TEST_NAME} ${TName})
ENDFOREACH(TEST)
