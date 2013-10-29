# Seed Grow Segmentation Filter Tests
create_test_sourcelist(TEST_SOURCES SGSF_Tests.cpp # this file is created by this command
  seed_grow_segmentation_simple_execution.cpp
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(SGSF_Tests 
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${FILTERS_DIR}/SeedGrowSegmentationFilter.cpp
  seed_grow_segmentation_testing_support.cpp
#   ${CORE_DIR}/Analysis/Channel.cpp
#   ${CORE_DIR}/Analysis/ViewItem.cpp
#   ${GUI_DIR}/ModelFactory.cpp
#   ${GUI_DIR}/Model/ChannelAdapter.cpp
  )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${LIB_DEPENDECIES}
)


target_link_libraries(SGSF_Tests ${LIB_DEPENDECIES} )

add_test("\"Seed Grow Segmentation Filter: Simple Execution\"" SGSF_Tests seed_grow_segmentation_simple_execution)