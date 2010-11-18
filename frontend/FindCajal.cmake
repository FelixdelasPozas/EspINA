# Try to find libCajalXXX
# Once done, this will define:
# 
# Cajal_FOUND: System has Cajal libraries installed
# Cajal_INCLUDE_DIR: The Cajal include directories
# Cajal_LIBRARIES: The Cajal libraries
# Cajal_GUI_LIBRARIES: The GUI Cajal libraries

FIND_PATH(Cajal_INCLUDE_DIR slice_view.h /usr/include/ /usr/local/include/)

FIND_LIBRARY(Cajal_LIBRARIES NAMES CajalGUI PATH /usr/lib /usr/local/lib) 

IF (Cajal_INCLUDE_DIR AND Cajal_LIBRARIES)
	   SET(Cajal_FOUND TRUE)
ENDIF (Cajal_INCLUDE_DIR AND Cajal_LIBRARIES)

IF (Cajal_FOUND)
	IF (NOT Cajal_FIND_QUIETLY)
		MESSAGE(STATUS "Found Cajal: ${Cajal_LIBRARIES}")
	ENDIF (NOT Cajal_FIND_QUIETLY)
ELSE (Cajal_FOUND)
	IF (Cajal_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find Cajal")
	ENDIF (Cajal_FIND_REQUIRED)
ENDIF (Cajal_FOUND)
