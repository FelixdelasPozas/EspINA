include_directories(
  ${CMAKE_BINARY_DIR}         # Generated EspinaConfig.h
  ${CMAKE_CURRENT_BINARY_DIR} #For wrap/ui files
)

include (GenerateExportHeader)

# qt4_wrap_cpp(FILTERS_MOC
#   Analysis/Data.h
#   Analysis/Output.h
#   MultiTasking/Scheduler.h
#   MultiTasking/Task.h
#   OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
#   )

set (FILTERS_SOURCES
  ${FILTERS_MOC}
  ${FILTERS_UI}
  SeedGrowSegmentationFilter.cpp
  )

set(FILTERS_EXTERNAL_LIBS
  ${QT_LIBRARIES}
#${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
)

if(DEFINED MINGW OR DEFINED MSVC)

  if (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (FILTERS_EXTERNAL_LIBS ${FILTERS_EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
  endif (CMAKE_BUILD_TYPE MATCHES Debug)

  if(DEFINED ESPINA_VERSION_MAJOR)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/windllprops.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc")
    set(FILTERS_SOURCES ${FILTERS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/windllprops.rc)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
    enable_language(RC)
  endif(DEFINED ESPINA_VERSION_MAJOR)

endif(DEFINED MINGW OR DEFINED MSVC)


add_library (EspinaFilters SHARED ${FILTERS_SOURCES})

GENERATE_EXPORT_HEADER(
  EspinaFilters
  BASE_NAME EspinaFilters
  EXPORT_MACRO_NAME EspinaFilters_EXPORT
  EXPORT_FILE_NAME EspinaFilters_Export.h
  )
target_link_libraries (EspinaFilters EspinaCore ${FILTERS_EXTERNAL_LIBS})