find_package (VTK REQUIRED NO_MODULE)
include (${VTK_USE_FILE})

include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_BINARY_DIR}/Filters # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

qt5_wrap_cpp(FILTERS_MOC
  MorphologicalEditionFilter.h
  SeedGrowSegmentationFilter.h
  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
  )

set (FILTERS_SOURCES
  ${FILTERS_MOC}
  ${FILTERS_UI}
  CleanSegmentationVoxelsFilter.cpp
  CloseFilter.cpp
  DilateFilter.cpp
  ErodeFilter.cpp
  FillHoles2DFilter.cpp
  FillHolesFilter.cpp
  ImageLogicFilter.cpp
  MorphologicalEditionFilter.cpp
  OpenFilter.cpp
  SeedGrowSegmentationFilter.cpp
  SliceInterpolationFilter.cpp
  SplitFilter.cpp
  CleanSegmentationVoxelsFilter.cpp
  Utils/vtkTube.cpp
  Utils/Stencil.cpp
  )

set(FILTERS_EXTERNAL_LIBS
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  EspinaExtensions
)

if(DEFINED MINGW)

  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (FILTERS_EXTERNAL_LIBS ${FILTERS_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(FILTERS_SOURCES ${FILTERS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)

endif(DEFINED MINGW)

add_library (EspinaFilters SHARED ${FILTERS_SOURCES})

target_link_libraries (EspinaFilters EspinaCore ${FILTERS_EXTERNAL_LIBS})

GENERATE_EXPORT_HEADER(
  EspinaFilters
  BASE_NAME EspinaFilters
  EXPORT_MACRO_NAME EspinaFilters_EXPORT
  EXPORT_FILE_NAME EspinaFilters_Export.h
  )
 
install(TARGETS EspinaFilters
  # IMPORTANT: Add the EspinaFilters library to the "export-set"
  EXPORT        ESPINATargets
  RUNTIME       DESTINATION "${INSTALL_BIN_DIR}"            COMPONENT bin
  LIBRARY       DESTINATION "${INSTALL_LIB_DIR}"            COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/espina" COMPONENT dev
  )

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE_DIR}/espina
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.cxx" PATTERN "*.txx")
