find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package (Qt4 REQUIRED)
if (QT4_FOUND)
	include (${QT_USE_FILE})
endif (QT4_FOUND)

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
link_directories ( ${Boost_LIBRARY_DIRS} )

set(LIB_DEPENDECIES
 	Espina
 	${QT_LIBRARIES}
        ${Boost_LIBRARIES}
  	pqComponents
        pqApplicationComponents
)

add_subdirectory(IOEspinaFile)
