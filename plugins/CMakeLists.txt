# add espina plugins below this line taking dependencies into account
# (if dependencies between plugins exists, less dependent first)

SET (BUILD_APPOSITON_SURFACE ON CACHE BOOL "Build Apposition Surface Plugin")
IF (BUILD_APPOSITON_SURFACE)
  add_subdirectory(AppositionSurface ${ESPINA_PLUGINS_DIR})
ENDIF (BUILD_APPOSITON_SURFACE)

SET (BUILD_SEGMHA_IMPORTER ON CACHE BOOL "Build Segmha Importer Plugin")
IF (BUILD_SEGMHA_IMPORTER)
  add_subdirectory(SegmhaImporter ${ESPINA_PLUGINS_DIR})
ENDIF (BUILD_SEGMHA_IMPORTER)


SET (BUILD_COUNTING_REGION ON CACHE BOOL "Build Counting Region Plugin")
IF (BUILD_COUNTING_REGION)
  add_subdirectory(CountingRegion ${ESPINA_PLUGINS_DIR})
ENDIF (BUILD_COUNTING_REGION)
