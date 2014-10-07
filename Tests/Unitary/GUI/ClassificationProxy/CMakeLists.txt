# ClassificationProxy tests
create_test_sourcelist(ClassificationProxy_Tests ClassificationProxy_Tests.cpp # this file is created by this command
  classification_proxy_add_categories.cpp
  classification_proxy_add_segmentation.cpp
  classification_proxy_add_segmentation_to_subcategory.cpp
  classification_proxy_add_segmentation_without_category.cpp
  classification_proxy_change_category_parent.cpp
  classification_proxy_change_segmentation_category.cpp
  classification_proxy_empty_analysis_constructor.cpp
  classification_proxy_remove_category.cpp
  classification_proxy_remove_segmentation.cpp
  classification_proxy_remove_segmentation_from_subcategory.cpp
  classification_proxy_rename_category.cpp
  classification_proxy_set_classification.cpp
  classification_proxy_set_default_classification.cpp
  classification_proxy_set_empty_classification.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

if (UNIX)
  set (LIB_DEPENDECIES
     ${LIB_DEPENDECIES}
     QtTest
    )
endif (UNIX)
qt4_add_resources(SUBJECT_RCC
  ${CORE_DIR}/rsc/core.qrc
  )

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
  ${GUI_DIR}/Model/ModelAdapter.h
  ${GUI_DIR}/Model/FilterAdapter.h
  ${GUI_DIR}/Model/ItemAdapter.h
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.h
  ModelTest.h
)

add_executable(ClassificationProxy_Tests
  ${ClassificationProxy_Tests}
  ${SUBJECT_RCC}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/IO/ClassificationXML.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp 
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${EXTENSIONS_DIR}/Notes/SegmentationNotes.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/CategoryAdapter.cpp
  ${GUI_DIR}/Model/ChannelAdapter.cpp
  ${GUI_DIR}/Model/ClassificationAdapter.cpp
  ${GUI_DIR}/Model/NeuroItemAdapter.cpp
  ${GUI_DIR}/Model/ModelAdapter.cpp
  ${GUI_DIR}/Model/Proxies/ClassificationProxy.cpp
  ${GUI_DIR}/Model/SampleAdapter.cpp
  ${GUI_DIR}/Model/SegmentationAdapter.cpp
  ${GUI_DIR}/Model/ViewItemAdapter.cpp
  ${GUI_DIR}/ModelFactory.cpp
  ${GUI_DIR}/Representations/RepresentationFactoryGroup.cpp
  ModelTest.cpp
  )
target_link_libraries(ClassificationProxy_Tests ${LIB_DEPENDECIES} )

add_test("\"Classification Proxy: Empty Analysis Constructor\""              ClassificationProxy_Tests classification_proxy_empty_analysis_constructor)
add_test("\"Classification Proxy: Set Empty Classification\""                ClassificationProxy_Tests classification_proxy_set_empty_classification)
add_test("\"Classification Proxy: Add Categories\""                          ClassificationProxy_Tests classification_proxy_add_categories)
add_test("\"Classification Proxy: Remove Category\""                         ClassificationProxy_Tests classification_proxy_remove_category)
add_test("\"Classification Proxy: Set Classification\""                      ClassificationProxy_Tests classification_proxy_set_classification)
add_test("\"Classification Proxy: Set Default Classification\""              ClassificationProxy_Tests classification_proxy_set_default_classification)
add_test("\"Classification Proxy: Add Segmentation\""                        ClassificationProxy_Tests classification_proxy_add_segmentation)
add_test("\"Classification Proxy: Add Segmentation To Subcategory\""         ClassificationProxy_Tests classification_proxy_add_segmentation_to_subcategory)
add_test("\"Classification Proxy: Add Segmentation Without Category\""       ClassificationProxy_Tests classification_proxy_add_segmentation_without_category)
add_test("\"Classification Proxy: Change Segmentation Category\""            ClassificationProxy_Tests classification_proxy_change_segmentation_category)
add_test("\"Classification Proxy: Remove Segmentation\""                     ClassificationProxy_Tests classification_proxy_remove_segmentation)
add_test("\"Classification Proxy: Remove Segmentation From Subcategory\""    ClassificationProxy_Tests classification_proxy_remove_segmentation_from_subcategory)
add_test("\"Classification Proxy: Rename Category \""                        ClassificationProxy_Tests classification_proxy_rename_category)
add_test("\"Classification Proxy: Change Category Parent\""                  ClassificationProxy_Tests classification_proxy_change_category_parent)
