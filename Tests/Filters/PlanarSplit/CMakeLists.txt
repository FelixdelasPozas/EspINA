# Planar Split Filter Tests
create_test_sourcelist(TEST_SOURCES Test.cpp # this file is created by this command
  planar_split_basic_pipeline.cpp
  planar_split_load_pipeline.cpp
  planar_split_restore_pipeline.cpp
)

set (SOURCES
  ${TEST_SOURCES}
  testing.cpp
)

add_executable(Test "" ${SOURCES} )


target_link_libraries(Test ${TESTING_DEPENDECIES} )

add_test("\"Planar Split: Basic Pipeline\""   Test planar_split_basic_pipeline)
add_test("\"Planar Split: Load Pipeline\""    Test planar_split_load_pipeline)
add_test("\"Planar Split: Restore Pipeline\"" Test planar_split_restore_pipeline)