if (ESPINA_FOUND)
  # Tell the compiler where to find ESPINA's header files
  include_directories (${ESPINA_INCLUDE_DIR})

  # Tell the linker where to find ESPINA's libraries
  link_directories (${ESPINA_LIBRARY_DIRECTORIES})

endif (ESPINA_FOUND)
