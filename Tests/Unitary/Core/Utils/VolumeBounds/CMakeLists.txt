# Volume Bound tests
create_test_sourcelist(VolumeBounds_Tests VolumeBounds_Tests.cpp # this file is created by this command
  default_constructor.cpp
  bounds_constructor.cpp
  comparison_operators.cpp
  compatibility.cpp
  equivalence.cpp
  length.cpp
  bounding_box.cpp
  intersection.cpp
  inclusion.cpp
  inclusion_exclusion.cpp
  stream_operator.cpp
  toString.cpp
)

set(LIB_DEPENDECIES
  ${ITK_LIBRARIES}
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)


add_executable(VolumeBounds_Tests
  ${VolumeBounds_Tests}
  ${CORE_DIR}/Utils/Bounds.cpp
  ${CORE_DIR}/Utils/NmVector3.cpp
  ${CORE_DIR}/Utils/VolumeBounds.cpp
  )
target_link_libraries(VolumeBounds_Tests ${LIB_DEPENDECIES} )

add_test("\"Volume Bounds: Default Constructor\"" VolumeBounds_Tests default_constructor)
add_test("\"Volume Bounds: Bounds Constructor\""  VolumeBounds_Tests bounds_constructor)
add_test("\"Volume Bounds: Comparison Operator\"" VolumeBounds_Tests comparison_operators)
add_test("\"Volume Bounds: Compatibility\""       VolumeBounds_Tests compatibility)
add_test("\"Volume Bounds: Equivalnce\""          VolumeBounds_Tests equivalence)
add_test("\"Volume Bounds: Length\""              VolumeBounds_Tests length)
add_test("\"Volume Bounds: Bounding Box\""        VolumeBounds_Tests bounding_box)
add_test("\"Volume Bounds: Intersection\""        VolumeBounds_Tests intersection)
add_test("\"Volume Bounds: Inclusion\""           VolumeBounds_Tests inclusion)
add_test("\"Volume Bounds: Inclusion Exclusion\"" VolumeBounds_Tests inclusion_exclusion)
add_test("\"Volume Bounds: toString\""            VolumeBounds_Tests toString)
add_test("\"Volume Bounds: Stream Operator\""     VolumeBounds_Tests stream_operator)