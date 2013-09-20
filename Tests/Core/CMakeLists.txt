# find_package(Quazip REQUIRED)
# include(${QUAZIP_USE_FILE})
# 
# find_package(Qt4 REQUIRED)
# include(${QT_USE_FILE})
# 
# find_package (VTK REQUIRED)
# include (${VTK_USE_FILE})
# 
# find_package (ITK REQUIRED)
# include (${ITK_USE_FILE})
# 
# find_package(Boost REQUIRED COMPONENTS  system filesystem graph)
# link_directories ( ${Boost_LIBRARY_DIRS} )
# 
set(LIB_DEPENDECIES
  EspinaCore
  EspinaGUI
  EspinaUndo
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  QVTK
)

if (NOT DEFINED MINGW AND NOT DEFINED MSVC)
  set(LIB_DEPENDECIES QtTest ${LIB_DEPENDECIES})
else (NOT DEFINED MINGW AND NOT DEFINED MSVC)
  set(LIB_DEPENDECIES  ${QT_QTTEST_LIBRARY} ${LIB_DEPENDECIES})
endif (NOT DEFINED MINGW AND NOT DEFINED MSVC)

add_subdirectory(IO)
# add_subdirectory(Model)
