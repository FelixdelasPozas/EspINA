# Segmentation tests
create_test_sourcelist(TEST_SOURCES Segmentation_Tests.cpp # this file is created by this command
  segmentation_default_constructor.cpp
  segmentation_save_state.cpp
  segmentation_restore_state.cpp
  segmentation_change_number.cpp
  segmentation_change_category.cpp
  segmentation_user_modifications.cpp
  segmentation_change_output.cpp
  segmentation_add_extension.cpp
  segmentation_delete_extension.cpp
  segmentation_request_extension_information.cpp
  segmentation_request_non_existing_information.cpp
  segmentation_unload.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")
set( TASK_DIR "${CORE_DIR}/MultiTasking")

include_directories(
  ${SUBJECT_DIR}
  )

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

set(SUBJECT_SOURCES
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Classification.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
 )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Segmentation_Tests
   ${SUBJECT_MOCS}
   ${SUBJECT_SOURCES}
  )

target_link_libraries(Segmentation_Tests ${LIB_DEPENDECIES} )

add_test("\"Segmentation: Default Constructor\""              Segmentation_Tests segmentation_default_constructor)
add_test("\"Segmentation: Save State\""                       Segmentation_Tests segmentation_save_state)
add_test("\"Segmentation: Restore State\""                    Segmentation_Tests segmentation_restore_state)
add_test("\"Segmentation: Change Number\""                    Segmentation_Tests segmentation_change_number)
add_test("\"Segmentation: Change Category\""                  Segmentation_Tests segmentation_change_category)
add_test("\"Segmentation: Register User Modifications\""      Segmentation_Tests segmentation_user_modifications)
add_test("\"Segmentation: Change Output\""                    Segmentation_Tests segmentation_change_output)
add_test("\"Segmentation: Add Extension\""                    Segmentation_Tests segmentation_add_extension)
add_test("\"Segmentation: Delete Extension\""                 Segmentation_Tests segmentation_delete_extension)
add_test("\"Segmentation: Request Extension Information\""    Segmentation_Tests segmentation_request_extension_information)
add_test("\"Segmentation: Request Non Existing Information\"" Segmentation_Tests segmentation_request_non_existing_information)
add_test("\"Segmentation: Unload\""                           Segmentation_Tests segmentation_unload)