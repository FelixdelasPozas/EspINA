# Bound tests
create_test_sourcelist(Bounds_Tests Bounds_Tests.cpp # this file is created by this command
  comparison_operators.cpp
  default_constructor.cpp
  inclusion.cpp
  invalid_intersection.cpp
  invalid_list_constructor.cpp
  length.cpp
  limit_inclusion_intersection.cpp
  list_constructor.cpp
  stream_operator.cpp
  valid_bounding_box.cpp
  valid_intersection.cpp
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
  ${SUBJECT_DIR}/NmVector3.cpp
  )
target_link_libraries(Bounds_Tests ${LIB_DEPENDECIES} )

add_test("\"Bounds: Default Constructor\""          Bounds_Tests default_constructor)
add_test("\"Bounds: List Constructor\""             Bounds_Tests list_constructor)
add_test("\"Bounds: Invalid List Constructor\""     Bounds_Tests invalid_list_constructor)
add_test("\"Bounds: Comparison Operator\""          Bounds_Tests comparison_operators)
add_test("\"Bounds: Length\""                       Bounds_Tests length)
add_test("\"Bounds: Valid Bounding Box\""           Bounds_Tests valid_bounding_box)
add_test("\"Bounds: Valid Intersection\""           Bounds_Tests valid_intersection)
add_test("\"Bounds: Invalid Intersection\""         Bounds_Tests invalid_intersection)
add_test("\"Bounds: Limit Inclusion Intersection\"" Bounds_Tests limit_inclusion_intersection)
add_test("\"Bounds: Inclusion\""                    Bounds_Tests inclusion)
add_test("\"Bounds: Stream Operator\""              Bounds_Tests stream_operator)