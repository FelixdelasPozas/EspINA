set(CORE_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${TESTING_DEPENDECIES}
)

add_subdirectory( Analysis )
add_subdirectory( Data )
add_subdirectory( Channel )
add_subdirectory( Classification )
add_subdirectory( CoreFactory )
add_subdirectory( DirectedGraph )
add_subdirectory( Filter )
add_subdirectory( Output )
add_subdirectory( Query )
add_subdirectory( Scheduler )
add_subdirectory( Sample )
add_subdirectory( Segmentation )
add_subdirectory( Utils )

