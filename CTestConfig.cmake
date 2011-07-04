## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard

# SET (CTEST_INITIAL_CACHE  "
#   CXXFLAGS:STRING=-g -O0 -Wall -Wshadow -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage
#   CFLAGS:STRING=-g -O0 -Wall -W -fprofile-arcs -ftest-coverage
#   LDFLAGS:STRING=-fprofile-arcs -ftest-coverage
#   "
#   )
set(CTEST_PROJECT_NAME "libcajal")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "bb13.cesvima.upm.es")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=libcajal")
set(CTEST_DROP_SITE_CDASH TRUE)
