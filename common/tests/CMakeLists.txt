find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

find_package (Qt4 REQUIRED)
if (QT4_FOUND)
	include (${QT_USE_FILE})
endif (QT4_FOUND)

find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

set(TESTS 
	#ProcessingTrace
#   RemoteFile
  FilePack
)

include_directories (
  ..
  ${PARAVIEWFILES}
)

set(LIB_DEPENDECIES
 	Espina
 	${QT_LIBRARIES}
        ${Boost_LIBRARIES}
  	pqComponents
        pqApplicationComponents
)

#SET (STACK_DIR "" CACHE PATH "Path to test Stack's folder")

foreach(TEST ${TESTS})
	set(EXE_NAME Espina${TEST})
	add_executable(${EXE_NAME} Test${TEST}.cpp)
	target_link_libraries(${EXE_NAME} ${LIB_DEPENDECIES})
	add_test(Espina-${TEST} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME})
endforeach(TEST)

#add_subdirectory(IOEspinaFile)
