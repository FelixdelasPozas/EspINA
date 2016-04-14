# StreamedVolume tests
create_test_sourcelist(StreamedVolume_Tests StreamedVolume_Tests.cpp # this file is created by this command
  streamedVolume_constructors.cpp
  streamedVolume_writeread.cpp
  streamedVolume_concurrent_writeread.cpp
  )

add_executable(StreamedVolume_Tests "" ${StreamedVolume_Tests} )
target_link_libraries(StreamedVolume_Tests ${CORE_DEPENDECIES} )

add_test("\"Streamed Files: StreamedVolume/WritableStreamedVolume Constructors\""      StreamedVolume_Tests streamedVolume_constructors)
add_test("\"Streamed Files: WritableStreamedVolume Write/Read & StreamedVolume Read\"" StreamedVolume_Tests streamedVolume_writeread)
add_test("\"Streamed Files: WritableStreamedVolume Concurrent Write/Read\""            StreamedVolume_Tests streamedVolume_concurrent_writeread)
