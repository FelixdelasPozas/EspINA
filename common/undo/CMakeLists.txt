set (CORE_SOURCES
  ${CORE_SOURCES}
  undo/AddChannel.cpp
  undo/AddFilter.cpp
  undo/AddRelation.cpp
  undo/AddSample.cpp
  undo/AddSegmentation.cpp
  undo/RemoveSegmentation.cpp
  PARENT_SCOPE
  )

add_library(undo_fake
  AddChannel.cpp
  AddFilter.cpp
  AddRelation.cpp
  AddSample.cpp
  AddSegmentation.cpp
  RemoveSegmentation.cpp
  )