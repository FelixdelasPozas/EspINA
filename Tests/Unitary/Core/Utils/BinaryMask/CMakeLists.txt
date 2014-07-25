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

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

qt4_wrap_cpp(SUBJECT_MOCS
)

add_executable(BinaryMask_Tests
  ${BinaryMask_Tests}
#  ${SUBJECT_MOCS}
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  )

target_link_libraries(BinaryMask_Tests ${LIB_DEPENDECIES} )

add_test("\"Binary Mask: Default Constructor\""                     BinaryMask_Tests default_constructor)
add_test("\"Binary Mask: Modify Mask\""                             BinaryMask_Tests modify_mask)
add_test("\"Binary Mask: Iterator\""                                BinaryMask_Tests iterator)
add_test("\"Binary Mask: Const Iterator\""                          BinaryMask_Tests const_iterator)
add_test("\"Binary Mask: Region Iterator\""                         BinaryMask_Tests region_iterator)
add_test("\"Binary Mask: Region Const Iterator\""                   BinaryMask_Tests region_const_iterator)
add_test("\"Binary Mask: ItkImage Dump\""                           BinaryMask_Tests itkImage_dump)
add_test("\"Binary Mask: Constructor for a given image\""           BinaryMask_Tests itkImage_constructor)
