find_package (Qt4 REQUIRED)

if (QT4_FOUND)
   include (${QT_USE_FILE})
endif (QT4_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

include_directories(
	.
	cache
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
FILE ( GLOB CACHE_SOURCES cache/*.cpp )

SET(Espina_SOURCE_FILES
  ${LOCAL_SOURCES}
  ${CACHE_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${RCC_BUILT_SOURCES}
  )


add_library (Espina SHARED ${Espina_SOURCE_FILES})

#target_link_libraries (espina ${Cajal_LIBRARIES})
