# Analysis tests
create_test_sourcelist(IO_Tests IO_Tests.cpp # this file is created by this command
  io_analysis_seg_file_io.cpp
  io_load_seg_file_analysis_without_registered_filters.cpp
  io_classification_xml_io.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(IO_Tests
  ${IO_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Data/MeshData.cpp
  ${CORE_DIR}/Analysis/Data/Mesh/RawMesh.cpp
  ${CORE_DIR}/Analysis/Data/VolumetricData.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/IO/ClassificationXML.cpp
  ${CORE_DIR}/IO/FetchRawData.cpp
  ${CORE_DIR}/IO/GraphIO.cpp
  ${CORE_DIR}/IO/SegFile.cpp
  ${CORE_DIR}/IO/SegFileInterface.cpp
  ${CORE_DIR}/IO/SegFile_V4.cpp
  ${CORE_DIR}/IO/SegFile_V5.cpp
  ${CORE_DIR}/IO/ReadOnlyChannelExtension.cpp
  ${CORE_DIR}/IO/ReadOnlySegmentationExtension.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/TemporalStorage.cpp
  ${CORE_DIR}/Utils/vtkPolyDataUtils.cpp
  io_testing_support.cpp
  )
target_link_libraries(IO_Tests ${LIB_DEPENDECIES} )

add_test("\"IO: Classification XML IO\""                     IO_Tests io_classification_xml_io)
add_test("\"IO: Analysis Seg File IO\""                      IO_Tests io_analysis_seg_file_io)
add_test("\"IO: Load Seg File Without Registered Filters\""  IO_Tests io_load_seg_file_analysis_without_registered_filters)