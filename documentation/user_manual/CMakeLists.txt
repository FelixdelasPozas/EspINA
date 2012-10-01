# CMakeLists.txt, 2009-04-15 -- Bayesian Programming of a Micro-Helicopter
# @Last Change: 2009-04-15
# vi: 
# Local Variables:
# End:

PROJECT( Rapport-StageMOSIG NONE )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE( UseLATEX.cmake )

SET( PDFLATEX_COMPILER pdflatex ) 

#Search for Input files:
FILE(GLOB rapport_inputs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.tex")
MESSAGE(STATUS "Found Tex Files: " ${rapport_inputs} ) 
#Search for packages files:
FILE(GLOB rapport_stys RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.sty")
MESSAGE(STATUS "Found sty Files: " ${rapport_stys} ) 
#Search for Bib files:
FILE(GLOB rapport_bib RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.bib")
MESSAGE(STATUS "Found Bib Files: " ${rapport_bib} ) 
#Search for asy files:
FILE(GLOB rapport_asys RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.asy")
MESSAGE(STATUS "Found sty Files: " ${rapport_asys} ) 


#Basically it's going to copy all required files to the build dir
ADD_LATEX_DOCUMENT( espina-user-manual.tex  #your main file
		BIBFILES
		  ${rapport_bib}
		INPUTS 
		  ${rapport_inputs}
		  ${rapport_stys}
		  ${rapport_asys}
		IMAGE_DIRS
		  fig
		  ../../frontend/rsc
		  ../../frontend/toolbar/seedgrow/rsc
		  ../../frontend/toolbar/voi/rsc
		DEFAULT_PDF
		)
