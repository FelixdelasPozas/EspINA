create_test_sourcelist(TEST_SOURCES ClassificationAdapter_Tests.cpp # this file is created by this command
  classification_adapter_create_category.cpp
  classification_adapter_create_existing_category.cpp
  classification_adapter_change_name.cpp
  classification_adapter_change_existing_name.cpp
  classification_adapter_remove_category.cpp
  classification_adapter_remove_subcategory.cpp
  classification_adapter_adapt_existing_classification.cpp
  classification_adapter_classification_sync.cpp
)

include_directories(
  ${SUBJECT_DIR}
  )

add_executable(ClassificationAdapter_Tests ${TEST_SOURCES} classification_adapter_testing_support.cpp )

target_link_libraries(ClassificationAdapter_Tests ${GUI_DEPENDECIES} )

add_test("\"Classification Adapter: Create Category Adapter\""       ClassificationAdapter_Tests classification_adapter_create_category)
add_test("\"Classification Adapter: Create Existing Category\""      ClassificationAdapter_Tests classification_adapter_create_existing_category)
add_test("\"Classification Adapter: Change Category Name\""          ClassificationAdapter_Tests classification_adapter_change_name)
add_test("\"Classification Adapter: Change Category Existing Name\"" ClassificationAdapter_Tests classification_adapter_change_existing_name)
add_test("\"Classification Adapter: Remove Category\""               ClassificationAdapter_Tests classification_adapter_remove_category)
add_test("\"Classification Adapter: Remove Sub-Category\""           ClassificationAdapter_Tests classification_adapter_remove_subcategory)
add_test("\"Classification Adapter: Adapt Existing Classification\"" ClassificationAdapter_Tests classification_adapter_adapt_existing_classification)
add_test("\"Classification Adapter: ClassificationAdapter sync with adapted Classification\"" ClassificationAdapter_Tests classification_adapter_classification_sync)