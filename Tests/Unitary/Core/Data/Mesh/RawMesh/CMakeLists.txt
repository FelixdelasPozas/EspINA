create_test_sourcelist(RawMesh_Tests RawMesh_Tests.cpp # this file is created by this command
  raw_mesh_default_constructor.cpp
)

add_executable(RawMesh_Tests "" ${RawMesh_Tests})

target_link_libraries(RawMesh_Tests ${CORE_DEPENDECIES})

add_test("\"Raw Mesh: Default Constructor\"" RawMesh_Tests raw_mesh_default_constructor)