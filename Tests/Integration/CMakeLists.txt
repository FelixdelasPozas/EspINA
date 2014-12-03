set(TESTING_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${TESTING_DEPENDECIES}
)

add_subdirectory( Core )

#add_subdirectory(Metadona)