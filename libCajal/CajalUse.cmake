if (CAJAL_FOUND)
  # Tell the compiler where to find Cajal's header files
  include_directories (${CAJAL_INCLUDE_DIR})

  # Tell the linker where to find Cajal's libraries
  # link_directories (${C}
endif (CAJAL_FOUND)
