SET(TESTS 
#	CreateGraph
#	ProcessingTrace
	Taxonomy
)

# find_package (Qt4 REQUIRED)
# if (QT4_FOUND)
# 	include (${QT_USE_FILE})
# endif (QT4_FOUND)

# Include libCajal header files
INCLUDE_DIRECTORIES (..)

ADD_DEFINITIONS(
    ${QT_DEFINITIONS}
)

SET(LIB_DEPENDECIES
#	igraph
	CajalData
	${QT_LIBRARIES}
)

#SET (STACK_DIR "" CACHE PATH "Path to test Stack's folder")

FOREACH(TEST ${TESTS})
	SET(EXE_NAME Cajal${TEST})
	ADD_EXECUTABLE(${EXE_NAME} Test${TEST}.cpp)
	TARGET_LINK_LIBRARIES(${EXE_NAME} ${LIB_DEPENDECIES})
	ADD_TEST(Cajal-${TEST} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME})
ENDFOREACH(TEST)

