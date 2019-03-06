include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

# Add Qt Resource files
qt5_add_resources(EXTENSIONS_RCC
  EdgeDistances/rsc/EdgeDistances.qrc
  )

qt5_wrap_cpp(EXTENSIONS_MOC
  EdgeDistances/ChannelEdges.h
  SLIC/StackSLIC.h
#   OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (EXTENSIONS_SOURCES
  ${EXTENSIONS_MOC}
  ${EXTENSIONS_UI}
  ${EXTENSIONS_RCC}
  EdgeDistances/ChannelEdges.cpp
  EdgeDistances/ChannelEdgesFactory.cpp
  Issues/Issues.cpp
  Issues/SegmentationIssues.cpp
  Issues/SegmentationIssuesFactory.cpp
  EdgeDistances/AdaptiveEdgesCreator.cpp
  EdgeDistances/EdgesAnalyzer.cpp
  EdgeDistances/EdgeDistance.cpp
  EdgeDistances/EdgeDistanceFactory.cpp
  Morphological/MorphologicalInformation.cpp
  Morphological/MorphologicalInformationFactory.cpp
  Notes/SegmentationNotes.cpp
  Notes/SegmentationNotesFactory.cpp
  Tags/SegmentationTags.cpp
  Tags/SegmentationTagsFactory.cpp
  SkeletonInformation/AxonInformation.cpp
  SkeletonInformation/DendriteInformation.cpp
  SkeletonInformation/SynapseInformation.cpp
  SkeletonInformation/SkeletonInformationFactory.cpp
  SLIC/StackSLIC.cpp
  SLIC/StackSLICFactory.cpp
  BasicInformation/BasicSegmentationInformation.cpp
  BasicInformation/BasicSegmentationInformationFactory.cpp
  LibraryExtensionFactory.cpp
  )

set(EXTENSIONS_EXTERNAL_LIBS
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW)
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (EXTENSIONS_EXTERNAL_LIBS ${EXTENSIONS_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(EXTENSIONS_SOURCES ${EXTENSIONS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)
endif(DEFINED MINGW)

add_library (EspinaExtensions SHARED ${EXTENSIONS_SOURCES})

target_link_libraries (EspinaExtensions EspinaCore ${EXTENSIONS_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
  EspinaExtensions
  BASE_NAME EspinaExtensions
  EXPORT_MACRO_NAME EspinaExtensions_EXPORT
  EXPORT_FILE_NAME EspinaExtensions_Export.h
  )

install(TARGETS EspinaExtensions
  # IMPORTANT: Add the EspinaExtensions library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")
