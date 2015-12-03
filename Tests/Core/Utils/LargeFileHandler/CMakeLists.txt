# Large file handling tests
create_test_sourcelist(LargeFileHandler_Tests LargeFileHandler_Tests.cpp # this file is created by this command
  write_constructor.cpp
  read_constructor.cpp
  read.cpp
  write.cpp
  concurrent_read.cpp
  concurrent_write.cpp
  attributes.cpp
)

set(SUBJECT_DIR ${CORE_DIR}/Utils)

include_directories(
  ${SUBJECT_DIR}
  )

add_executable(LargeFileHandler_Tests "" ${LargeFileHandler_Tests} )
target_link_libraries(LargeFileHandler_Tests ${CORE_DEPENDECIES} )

add_test("\"Large Files: Write Constructor\"" LargeFileHandler_Tests write_constructor)
add_test("\"Large Files: Read Constructor\""  LargeFileHandler_Tests read_constructor)
add_test("\"Large Files: Read\""              LargeFileHandler_Tests read)
add_test("\"Large Files: Write\""             LargeFileHandler_Tests write)
add_test("\"Large Files: Concurrent Read\""   LargeFileHandler_Tests concurrent_read)
add_test("\"Large Files: Concurrent Write\""  LargeFileHandler_Tests concurrent_write)
add_test("\"Large Files: Attributes\""        LargeFileHandler_Tests attributes)
