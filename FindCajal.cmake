# Try to find libCajalXXX
# Once done, this will define:
# 
# CAJAL_FOUND: System has Cajal libraries installed
# CAJAL_INCLUDE_DIR: The Cajal include directories
# CAJAL_LIBRARIES: The Cajal libraries
# CAJAL_GUI_LIBRARIES: The GUI Cajal libraries
# Name conventions: Dependencies use plural forms, 
# the package itself uses the singular forms defined
# by find_path and find_library

FIND_PATH(CAJAL_INCLUDE_DIR taxonomy.h /usr/include/ /usr/local/include/)

FIND_LIBRARY(CAJAL_FILTER_LIBRARY NAMES CajalFilter PATH /usr/lib /usr/local/lib) 
FIND_LIBRARY(CAJAL_DATA_LIBRARY NAMES CajalData PATH /usr/lib /usr/local/lib) 

SET (CAJAL_INCLUDE_DIRS ${CAJAL_INCLUDE_DIR})
SET (CAJAL_LIBRARIES ${CAJAL_FILTER_LIBRARY} ${CAJAL_DATA_LIBRARY})

IF (CAJAL_INCLUDE_DIR AND CAJAL_LIBRARIES)
	   SET(CAJAL_FOUND TRUE)
ENDIF (CAJAL_INCLUDE_DIR AND CAJAL_LIBRARIES)

IF (CAJAL_FOUND)
	IF (NOT CAJAL_FIND_QUIETLY)
		MESSAGE(STATUS "Found Cajal: ${CAJAL_LIBRARIES}")
	ENDIF (NOT CAJAL_FIND_QUIETLY)
ELSE (CAJAL_FOUND)
	IF (CAJAL_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find Cajal")
	ENDIF (CAJAL_FIND_REQUIRED)
ENDIF (CAJAL_FOUND)

MARK_AS_ADVANCED(CAJAL_INCLUDE_DIR CAJAL_FILTER_LIBRARY CAJAL_DATA_LIBRARY)
