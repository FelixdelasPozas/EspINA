if (ESPINA_FOUND)
  # Tell the compiler where to find Espina's header files
  include_directories (${ESPINA_INCLUDE_DIR})

  # Tell the linker where to find Cajal's libraries
  # link_directories (${C}
endif (ESPINA_FOUND)