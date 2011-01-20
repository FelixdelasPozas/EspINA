project (CAJAL)

cmake_minimum_required (VERSION 2.6)

# Version Number
set (CAJAL_VERSION_MAJOR 0)
set (CAJAL_VERSION_MINOR 2)

#Has to be in the CMakeLists that declares a project
#may subprojects should be done and thus this move to
#the corresponding CMakeLists
ENABLE_TESTING()

add_subdirectory (filters)
add_subdirectory (data)
#add_subdirectory (gui)
