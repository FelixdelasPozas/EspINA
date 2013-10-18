# Bound tests
create_test_sourcelist(TEST_SOURCES DirectedGraph_Tests.cpp # this file is created by this command
  directed_graph_add_sample.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis")

include_directories(
  ${SUBJECT_DIR}
  )

# qt4_wrap_cpp(SUBJECT_MOCS
#   ${SUBJECT_DIR}/Category.h
#   ${SUBJECT_DIR}/Data.h
# )

# set(SUBJECT_SOURCES
#   ${SUBJECT_DIR}/Classification.cpp
#   ${SUBJECT_DIR}/Category.cpp
# )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(DirectedGraph_Tests
  ${TEST_SOURCES}
#   ${SUBJECT_MOCS}
#   ${SUBJECT_SOURCES}
  )

target_link_libraries(DirectedGraph_Tests ${LIB_DEPENDECIES} )

add_test("\"DirectedGraph: Add Item\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Remove Item\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Add Relation\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Remove Relation\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Add Existing Item\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Remove Non Existing Item\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Add Existing Relation\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Remove Non Existing Relation\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Add Relation Between Non Existing Items\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Input Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Output Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Filtered Input Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Filtered Output Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Filtered Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Vertex Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Remove Vertex Edges\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Vertex Ancestors\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Vertex Succesors\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Filtered Vertex Ancestors\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Filtered Vertex Succesors\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Expected Vertices\"" DirectedGraph_Tests directed_graph_)
add_test("\"DirectedGraph: Clear\"" DirectedGraph_Tests directed_graph_)