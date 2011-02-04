# Try to find libCajalXXX
# Once done, this will define:
# 
# CAJAL_FOUND: System has Cajal libraries installed
# CAJAL_INCLUDE_DIR: The Cajal include directories
# CAJAL_LIBRARIES: The Cajal libraries
# CAJAL_GUI_LIBRARIES: The GUI Cajal libraries

FIND_PATH(CAJAL_INCLUDE_DIR slice_view.h /usr/include/ /usr/local/include/)

FIND_LIBRARY(CAJAL_LIBRARIES NAMES CajalFilter CajalData PATH /usr/lib /usr/local/lib) 

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
