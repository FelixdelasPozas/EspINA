# add espina plugins below this line taking dependencies into account
# (if dependencies between plugins exists, less dependent first)
add_subdirectory(AppositionPlane ${ESPINA_PLUGINS_DIR})
add_subdirectory(SegmhaImporter ${ESPINA_PLUGINS_DIR})
add_subdirectory(CountingRegion ${ESPINA_PLUGINS_DIR})
