# Analysis tests
create_test_sourcelist(IO_Tests IO_Tests.cpp # this file is created by this command
  #io_analysis_seg_file_io.cpp
  io_classification_xml_io.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(IO_Tests
  ${IO_Tests}
  ${SUBJECT_MOCS}
  ${CORE_DIR}/IO/ClassificationXML.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Classification.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  )
target_link_libraries(IO_Tests ${LIB_DEPENDECIES} )

add_test("\"IO: Classification XML IO\"" IO_Tests io_classification_xml_io)
#add_test("\"IO: Analysis Seg File IO\""  IO_Tests io_analysis_seg_file_io)