# add espina plugins below this line taking dependencies into account
# (if dependencies between plugins exists, less dependent first)

set (LIBRARY_OUTPUT_PATH ${ESPINA_PLUGINS_DIR})

set(BUILD_APPOSITON_SURFACE ON CACHE BOOL "Build Apposition Surface Plugin")
if (BUILD_APPOSITON_SURFACE)

  add_subdirectory( AppositionSurface )

endif()

set (BUILD_SEGMHA_IMPORTER ON CACHE BOOL "Build Segmha Importer Plugin")
if (BUILD_SEGMHA_IMPORTER)

  add_subdirectory( SegmhaImporter )

endif()


set (BUILD_COUNTING_FRAME ON CACHE BOOL "Build Counting Region Plugin")
if (BUILD_COUNTING_FRAME)

  add_subdirectory( CountingFrame )

endif()
