# Binary Mask tests
create_test_sourcelist(BinaryMask_Tests BinaryMask_Tests.cpp # this file is created by this command
  binaryMask_default_constructor.cpp
  binaryMask_modify_mask.cpp
  binaryMask_iterator.cpp
  binaryMask_const_iterator.cpp
  binaryMask_region_iterator.cpp
  binaryMask_region_const_iterator.cpp
  binaryMask_itkImage_dump.cpp
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
  ${CORE_DIR}/Utils/BinaryMask.cpp
  ${CORE_DIR}/Utils/Bounds.cpp
  )
  
target_link_libraries(BinaryMask_Tests ${LIB_DEPENDECIES} )

add_test("\"Binary Mask: Default Constructor\""                     BinaryMask_Tests binaryMask_default_constructor)
add_test("\"Binary Mask: Modify Mask\""                             BinaryMask_Tests binaryMask_modify_mask)
add_test("\"Binary Mask: Iterator\""                                BinaryMask_Tests binaryMask_iterator)
add_test("\"Binary Mask: Const Iterator\""                          BinaryMask_Tests binaryMask_const_iterator)
add_test("\"Binary Mask: Region Iterator\""                         BinaryMask_Tests binaryMask_region_iterator)
add_test("\"Binary Mask: Region Const Iterator\""                   BinaryMask_Tests binaryMask_region_const_iterator)
add_test("\"Binary Mask: ItkImage Dump\""                           BinaryMask_Tests binaryMask_itkImage_dump)