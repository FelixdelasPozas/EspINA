# Analysis tests
create_test_sourcelist(Pipeline_Tests Pipeline_Tests.cpp # this file is created by this command
  pipeline_update_input.cpp
  pipeline_update_request_non_edited_input.cpp
  pipeline_update_request_edited_input.cpp
)

qt4_add_resources(SUBJECT_RCC
  ${CORE_DIR}/rsc/core.qrc
  )

qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/Analysis/Data.h
  ${CORE_DIR}/Analysis/Output.h
  ${CORE_DIR}/Analysis/Extension.h
  ${CORE_DIR}/Analysis/ViewItem.h
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

add_executable(Pipeline_Tests
  ${Pipeline_Tests}
  ${SUBJECT_MOCS}
  ${SUBJECT_RCC}
  ${CORE_DIR}/Analysis/Analysis.cpp
  ${CORE_DIR}/Analysis/Category.cpp
  ${CORE_DIR}/Analysis/Channel.cpp
  ${CORE_DIR}/Analysis/Data.cpp
  ${CORE_DIR}/Analysis/Data/MeshData.cpp
  ${CORE_DIR}/Analysis/Data/Mesh/RawMesh.cpp
  ${CORE_DIR}/Analysis/Data/VolumetricData.cpp
  ${CORE_DIR}/Analysis/Data/Volumetric/ROI.cpp
  ${CORE_DIR}/Analysis/Filter.cpp
  ${CORE_DIR}/Analysis/Graph/DirectedGraph.cpp
  ${CORE_DIR}/Analysis/Input.cpp
  ${CORE_DIR}/Analysis/Output.cpp
  ${CORE_DIR}/Analysis/Sample.cpp
  ${CORE_DIR}/Analysis/Segmentation.cpp
  ${CORE_DIR}/Analysis/ViewItem.cpp
  ${CORE_DIR}/Factory/CoreFactory.cpp
  ${CORE_DIR}/IO/ClassificationXML.cpp
  ${CORE_DIR}/IO/FetchBehaviour/FetchRawData.cpp
  ${CORE_DIR}/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.cpp
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
  ${FILTERS_DIR}/MorphologicalEditionFilter.cpp
  ${FILTERS_DIR}/DilateFilter.cpp
  ${FILTERS_DIR}/SeedGrowSegmentationFilter.cpp
  )

target_link_libraries(Pipeline_Tests ${TESTING_DEPENDECIES})

add_test("\"Pipeline: Update Input\""                    Pipeline_Tests pipeline_update_input)
add_test("\"Pipeline: Update Request Non Edited Input\"" Pipeline_Tests pipeline_update_request_non_edited_input)
add_test("\"Pipeline: Update Request Edited Input\""     Pipeline_Tests pipeline_update_request_edited_input)