include_directories(
  .
  ${CORE_DIR}/Utils
  )

set(LIB_DEPENDECIES
  )

add_subdirectory( AnalysisMerge    )
add_subdirectory( BinaryMask       )
add_subdirectory( Bounds           )
add_subdirectory( VolumeBounds     )
