# - Try to find xlslib
# Once done this will define: 
#  XLSLIB_FOUND - System has xlslib (Note: library file is actually called libxls)
#  XLSLIB_INCLUDE_DIRS - The xlslib inclut
#  XLSLIB_LIBRARIES - The libraries needed to use xlslib

find_package(PkgConfig)
pkg_check_modules(PC_XLSLIB QUIET libxls)

set(XLSLIB_DEFINITIONS ${PC_XLSLIB_CFLAGS_OTHER})

find_path(XLSLIB_INCLUDE_DIR xlslib.h
          HINTS ${PC_XLSLIB_INCLUDEDIR} ${PC_XLSLIB_INCLUDE_DIRS}
          PATH_SUFFIXES xlslib)

find_library(XLSLIB_LIBRARY NAMES xls libxls
             HINTS ${PC_XLSLIB_LIBDIR} ${PC_XLSLIB_LIBRARY_DIRS} )

set(XLSLIB_LIBRARIES ${XLSLIB_LIBRARY} )
set(XLSLIB_INCLUDE_DIRS ${XLSLIB_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set XLSLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(xlslib
                                  REQUIRED_VARS XLSLIB_LIBRARIES XLSLIB_INCLUDE_DIR)

#mark_as_advanced(XLSLIB_INCLUDE_DIR XLSLIB_LIBRARY )
