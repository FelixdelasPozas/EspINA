# Seed Grow Segmentation Filter Tests
create_test_sourcelist(TEST_SOURCES VolumetricStreamReader_Tests.cpp # this file is created by this command
  volumetric_stream_reader_simple_execution.cpp
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(VolumetricStreamReader_Tests 
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Data/VolumetricData.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${FILTERS_DIR}/VolumetricStreamReader.cpp
  )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  EspinaTesting
)


target_link_libraries(VolumetricStreamReader_Tests ${LIB_DEPENDECIES} )

add_test("\"Volumetric Stream Reader: Simple Execution\"" VolumetricStreamReader_Tests volumetric_stream_reader_simple_execution)