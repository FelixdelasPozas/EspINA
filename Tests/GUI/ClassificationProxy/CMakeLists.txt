# ClassificationProxy tests
create_test_sourcelist(ClassificationProxy_Tests ClassificationProxy_Tests.cpp # this file is created by this command
  classification_proxy_add_categories.cpp
  classification_proxy_add_segmentation.cpp
  classification_proxy_add_segmentation_group.cpp
  classification_proxy_add_segmentation_to_subcategory.cpp
  classification_proxy_add_segmentation_without_category.cpp
  classification_proxy_change_category_parent.cpp
  classification_proxy_change_segmentation_category.cpp
  classification_proxy_empty_analysis_constructor.cpp
  classification_proxy_remove_category.cpp
  classification_proxy_remove_segmentation.cpp
  classification_proxy_remove_segmentation_from_subcategory.cpp
  classification_proxy_remove_segmentation_group.cpp
  classification_proxy_rename_category.cpp
  classification_proxy_set_classification.cpp
  classification_proxy_set_default_classification.cpp
  classification_proxy_set_empty_classification.cpp
  classification_proxy_add_different_category_segmentations.cpp
  classification_proxy_change_segmentations_categories_to_different_categories.cpp
  classification_proxy_change_segmentations_categories_to_same_category.cpp
)

add_executable(ClassificationProxy_Tests "" ${ClassificationProxy_Tests})
target_link_libraries(ClassificationProxy_Tests ${TESTING_DEPENDECIES})

add_test("\"Classification Proxy: Empty Analysis Constructor\""                              ClassificationProxy_Tests classification_proxy_empty_analysis_constructor)
add_test("\"Classification Proxy: Set Empty Classification\""                                ClassificationProxy_Tests classification_proxy_set_empty_classification)
add_test("\"Classification Proxy: Add Categories\""                                          ClassificationProxy_Tests classification_proxy_add_categories)
add_test("\"Classification Proxy: Remove Category\""                                         ClassificationProxy_Tests classification_proxy_remove_category)
add_test("\"Classification Proxy: Set Classification\""                                      ClassificationProxy_Tests classification_proxy_set_classification)
add_test("\"Classification Proxy: Set Default Classification\""                              ClassificationProxy_Tests classification_proxy_set_default_classification)
add_test("\"Classification Proxy: Add Segmentation\""                                        ClassificationProxy_Tests classification_proxy_add_segmentation)
add_test("\"Classification Proxy: Add Segmentation Group\""                                  ClassificationProxy_Tests classification_proxy_add_segmentation_group)
add_test("\"Classification Proxy: Add Segmentation To Subcategory\""                         ClassificationProxy_Tests classification_proxy_add_segmentation_to_subcategory)
add_test("\"Classification Proxy: Add Segmentation Without Category\""                       ClassificationProxy_Tests classification_proxy_add_segmentation_without_category)
add_test("\"Classification Proxy: Add Different Category Segmentation\""                     ClassificationProxy_Tests classification_proxy_add_different_category_segmentations)
add_test("\"Classification Proxy: Change Segmentation Category\""                            ClassificationProxy_Tests classification_proxy_change_segmentation_category)
add_test("\"Classification Proxy: Change Segmentations Categories To Different Categories\"" ClassificationProxy_Tests classification_proxy_change_segmentations_categories_to_different_categories)
add_test("\"Classification Proxy: Change Segmentations Categories To Same Category\""        ClassificationProxy_Tests classification_proxy_change_segmentations_categories_to_same_category)
add_test("\"Classification Proxy: Remove Segmentation\""                                     ClassificationProxy_Tests classification_proxy_remove_segmentation)
add_test("\"Classification Proxy: Remove Segmentation Group\""                               ClassificationProxy_Tests classification_proxy_remove_segmentation_group)
add_test("\"Classification Proxy: Remove Segmentation From Subcategory\""                    ClassificationProxy_Tests classification_proxy_remove_segmentation_from_subcategory)
add_test("\"Classification Proxy: Rename Category \""                                        ClassificationProxy_Tests classification_proxy_rename_category)
add_test("\"Classification Proxy: Change Category Parent\""                                  ClassificationProxy_Tests classification_proxy_change_category_parent)
