include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

qt4_wrap_cpp(EXTENSIONS_MOC
  EdgeDistances/ChannelEdges.h
#   OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (EXTENSIONS_SOURCES
  ${EXTENSIONS_MOC}
  ${EXTENSIONS_UI}
  EdgeDistances/ChannelEdges.cpp
  EdgeDistances/AdaptiveEdgesCreator.cpp
  EdgeDistances/EdgesAnalyzer.cpp
  EdgeDistances/EdgeDistance.cpp
  Morphological/MorphologicalInformation.cpp
  Notes/SegmentationNotes.cpp
  Tags/SegmentationTags.cpp
  )

set(EXTENSIONS_EXTERNAL_LIBS
  ${QT_LIBRARIES}
#${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW OR DEFINED MSVC)

  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (EXTENSIONS_EXTERNAL_LIBS ${EXTENSIONS_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(EXTENSIONS_SOURCES ${EXTENSIONS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)

endif(DEFINED MINGW OR DEFINED MSVC)


add_library (EspinaExtensions SHARED ${EXTENSIONS_SOURCES})

target_link_libraries (EspinaExtensions EspinaCore ${EXTENSIONS_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
  EspinaExtensions
  BASE_NAME EspinaExtensions
  EXPORT_MACRO_NAME EspinaExtensions_EXPORT
  EXPORT_FILE_NAME EspinaExtensions_Export.h
  )

#set (EXTENSIONS_HEADERS 
#  #TODO: set all header files required to develop external apps
#  "${CMAKE_CURRENT_BINARY_DIR}/EspinaExtensions_Export.h"
#  )
#
#set_target_properties(EspinaExtensions PROPERTIES
#  PUBLIC_HEADER "${EXTENSIONS_HEADERS}")
 
install(TARGETS EspinaExtensions
  # IMPORTANT: Add the EspinaExtensions library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )
