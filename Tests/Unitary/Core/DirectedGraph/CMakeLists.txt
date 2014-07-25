# Bound tests
create_test_sourcelist(TEST_SOURCES DirectedGraph_Tests.cpp # this file is created by this command
  directed_graph_add_existing_item.cpp
  directed_graph_add_existing_relation.cpp
  directed_graph_add_item.cpp
  directed_graph_add_null_item.cpp
  directed_graph_add_relation.cpp
  directed_graph_add_relation_between_invalid_items.cpp
  directed_graph_add_relation_between_non_existing_items.cpp
  directed_graph_contains_item.cpp
  directed_graph_default_constructor.cpp
  directed_graph_expected_edges.cpp
  directed_graph_expected_filtered_edges.cpp
  directed_graph_expected_filtered_input_edges.cpp
  directed_graph_expected_filtered_output_edges.cpp
  directed_graph_expected_input_edges.cpp
  directed_graph_expected_output_edges.cpp
  directed_graph_remove_item.cpp
  directed_graph_remove_non_existing_item.cpp
  directed_graph_remove_non_existing_relation.cpp
  directed_graph_remove_relation.cpp
)

set( SUBJECT_DIR "${CORE_DIR}/Analysis/Graph")

include_directories(
  ${SUBJECT_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(DirectedGraph_Tests
  ${TEST_SOURCES}
  DummyItem.cpp
  ${SUBJECT_DIR}/DirectedGraph.cpp
  )

target_link_libraries(DirectedGraph_Tests ${LIB_DEPENDECIES} )

add_test("\"DirectedGraph: Default Constructor\""                     DirectedGraph_Tests directed_graph_default_constructor)
add_test("\"DirectedGraph: Add Null Item\""                           DirectedGraph_Tests directed_graph_add_null_item)
add_test("\"DirectedGraph: Add Item\""                                DirectedGraph_Tests directed_graph_add_item)
add_test("\"DirectedGraph: Contains Item\""                           DirectedGraph_Tests directed_graph_contains_item)
add_test("\"DirectedGraph: Remove Item\""                             DirectedGraph_Tests directed_graph_remove_item)
add_test("\"DirectedGraph: Add Relation\""                            DirectedGraph_Tests directed_graph_add_relation)
add_test("\"DirectedGraph: Remove Relation\""                         DirectedGraph_Tests directed_graph_remove_relation)
add_test("\"DirectedGraph: Add Existing Item\""                       DirectedGraph_Tests directed_graph_add_existing_item)
add_test("\"DirectedGraph: Remove Non Existing Item\""                DirectedGraph_Tests directed_graph_remove_non_existing_item)
add_test("\"DirectedGraph: Add Existing Relation\""                   DirectedGraph_Tests directed_graph_add_existing_relation)
add_test("\"DirectedGraph: Remove Non Existing Relation\""            DirectedGraph_Tests directed_graph_remove_non_existing_relation)
add_test("\"DirectedGraph: Add Relation Between Invalid Items\""      DirectedGraph_Tests directed_graph_add_relation_between_invalid_items)
add_test("\"DirectedGraph: Add Relation Between Non Existing Items\"" DirectedGraph_Tests directed_graph_add_relation_between_non_existing_items)
add_test("\"DirectedGraph: Expected Input Edges\""                    DirectedGraph_Tests directed_graph_expected_input_edges)
add_test("\"DirectedGraph: Expected Output Edges\""                   DirectedGraph_Tests directed_graph_expected_output_edges)
add_test("\"DirectedGraph: Expected Edges\""                          DirectedGraph_Tests directed_graph_expected_edges)
add_test("\"DirectedGraph: Expected Filtered Input Edges\""           DirectedGraph_Tests directed_graph_expected_filtered_input_edges)
add_test("\"DirectedGraph: Expected Filtered Output Edges\""          DirectedGraph_Tests directed_graph_expected_filtered_output_edges)
add_test("\"DirectedGraph: Expected Filtered Edges\""                 DirectedGraph_Tests directed_graph_expected_filtered_edges)
# add_test("\"DirectedGraph: Expected Vertex Edges\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Remove Vertex Edges\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Expected Vertex Ancestors\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Expected Vertex Succesors\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Expected Filtered Vertex Ancestors\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Expected Filtered Vertex Succesors\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Expected Vertices\"" DirectedGraph_Tests directed_graph_)
# add_test("\"DirectedGraph: Clear\"" DirectedGraph_Tests directed_graph_)