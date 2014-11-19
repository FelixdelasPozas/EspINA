# Binary Mask tests
create_test_sourcelist(BinaryMask_Tests BinaryMask_Tests.cpp # this file is created by this command
  default_constructor.cpp
  modify_mask.cpp
  iterator.cpp
  const_iterator.cpp
  region_iterator.cpp
  region_const_iterator.cpp
  itkImage_dump.cpp
  itkImage_constructor.cpp
)


add_executable(BinaryMask_Tests "" ${BinaryMask_Tests} )

target_link_libraries(BinaryMask_Tests ${CORE_DEPENDECIES} )

add_test("\"Binary Mask: Default Constructor\""                     BinaryMask_Tests default_constructor)
add_test("\"Binary Mask: Modify Mask\""                             BinaryMask_Tests modify_mask)
add_test("\"Binary Mask: Iterator\""                                BinaryMask_Tests iterator)
add_test("\"Binary Mask: Const Iterator\""                          BinaryMask_Tests const_iterator)
add_test("\"Binary Mask: Region Iterator\""                         BinaryMask_Tests region_iterator)
add_test("\"Binary Mask: Region Const Iterator\""                   BinaryMask_Tests region_const_iterator)
add_test("\"Binary Mask: ItkImage Dump\""                           BinaryMask_Tests itkImage_dump)
add_test("\"Binary Mask: Constructor for a given image\""           BinaryMask_Tests itkImage_constructor)
