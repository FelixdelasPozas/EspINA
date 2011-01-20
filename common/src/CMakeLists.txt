INCLUDE_DIRECTORIES(
	.
	
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
  )

#------------------------------------------------------------------------------
# Add Qt Resource files
QT4_ADD_RESOURCES(RCC_BUILT_SOURCES
	)
# Add extra library containing custom code for the client.
QT4_WRAP_CPP(MOC_BUILT_SOURCES
selectionManager.h
iPixelSelector.h
)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
)


FILE ( GLOB LOCAL_SOURCES *.cpp )

SET(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )


add_library (Espina SHARED ${Espina_SOURCE_FILES})

#target_link_libraries (espina ${Cajal_LIBRARIES})
