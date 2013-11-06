create_test_sourcelist(TEST_SOURCES Classification_Tests.cpp # this file is created by this command
  classification_create_category.cpp
  classification_create_existing_category.cpp
  classification_change_name.cpp
  classification_change_existing_name.cpp
  classification_remove_category.cpp
  classification_remove_subcategory.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")

include_directories(
  ${SUBJECT_DIR}
  )

qt4_wrap_cpp(SUBJECT_MOCS
  ${SUBJECT_DIR}/Category.h
)

set(SUBJECT_SOURCES
  ${SUBJECT_DIR}/Category.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Classification_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${SUBJECT_SOURCES}
  classification_testing_support.cpp
  )

target_link_libraries(Classification_Tests ${LIB_DEPENDECIES} )

add_test("\"Classification: Create Category\""               Classification_Tests classification_create_category)
add_test("\"Classification: Create Existing Category\""      Classification_Tests classification_create_existing_category)
add_test("\"Classification: Change Category Name\""          Classification_Tests classification_change_name)
add_test("\"Classification: Change Category Existing Name\"" Classification_Tests classification_change_existing_name)
add_test("\"Classification: Remove Category\""               Classification_Tests classification_remove_category)
add_test("\"Classification: Remove Sub-Category\""           Classification_Tests classification_remove_subcategory)