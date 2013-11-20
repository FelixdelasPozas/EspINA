# Bound tests
create_test_sourcelist(Bounds_Tests Bounds_Tests.cpp # this file is created by this command
  bounds_comparison_operators.cpp
  bounds_default_constructor.cpp
  bounds_inclusion.cpp
  bounds_invalid_intersection.cpp
  bounds_invalid_list_constructor.cpp
  bounds_length.cpp
  bounds_limit_inclusion_intersection.cpp
  bounds_list_constructor.cpp
  bounds_stream_operator.cpp
  bounds_valid_bounding_box.cpp
  bounds_valid_intersection.cpp
)

set(SUBJECT_DIR ${CORE_DIR}/Utils)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

include_directories(
  ${SUBJECT_DIR}
  )

add_executable(Bounds_Tests
  ${Bounds_Tests}
  ${SUBJECT_DIR}/Bounds.cpp
  )
target_link_libraries(Bounds_Tests ${LIB_DEPENDECIES} )

add_test("\"Bounds: Default Constructor\""          Bounds_Tests bounds_default_constructor)
add_test("\"Bounds: List Constructor\""             Bounds_Tests bounds_list_constructor)
add_test("\"Bounds: Invalid List Constructor\""     Bounds_Tests bounds_invalid_list_constructor)
add_test("\"Bounds: Comparison Operator\""          Bounds_Tests bounds_comparison_operators)
add_test("\"Bounds: Length\""                       Bounds_Tests bounds_length)
add_test("\"Bounds: Valid Bounding Box\""           Bounds_Tests bounds_valid_bounding_box)
add_test("\"Bounds: Valid Intersection\""           Bounds_Tests bounds_valid_intersection)
add_test("\"Bounds: Invalid Intersection\""         Bounds_Tests bounds_invalid_intersection)
add_test("\"Bounds: Limit Inclusion Intersection\"" Bounds_Tests bounds_limit_inclusion_intersection)
add_test("\"Bounds: Inclusion\""                    Bounds_Tests bounds_inclusion)
add_test("\"Bounds: Stream Operator\""              Bounds_Tests bounds_stream_operator)