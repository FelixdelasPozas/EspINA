create_test_sourcelist(TEST_SOURCES ClassificationAdapter_Tests.cpp # this file is created by this command
  classification_adapter_create_category.cpp
#   classification_adapter_create_existing_category.cpp
#   classification_adapter_change_name.cpp
#   classification_adapter_change_existing_name.cpp
#   classification_adapter_remove_category.cpp
#   classification_adapter_remove_subcategory.cpp
)

include_directories(
  ${SUBJECT_DIR}
  )

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Category.h
  ${GUI_DIR}/Model/ItemAdapter.h
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(ClassificationAdapter_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Category.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  classification_adapter_testing_support.cpp
  )

target_link_libraries(ClassificationAdapter_Tests ${LIB_DEPENDECIES} )

add_test("\"Classification Adapter: Create Category Adapter\""               ClassificationAdapter_Tests classification_adapter_create_category)
# add_test("\"Classification: Create Existing Category\""      ClassificationAdapter_Tests classification_adapter_create_existing_category)
# add_test("\"Classification: Change Category Name\""          ClassificationAdapter_Tests classification_adapter_change_name)
# add_test("\"Classification: Change Category Existing Name\"" ClassificationAdapter_Tests classification_adapter_change_existing_name)
# add_test("\"Classification: Remove Category\""               ClassificationAdapter_Tests classification_adapter_remove_category)
# add_test("\"Classification: Remove Sub-Category\""           ClassificationAdapter_Tests classification_adapter_remove_subcategory)