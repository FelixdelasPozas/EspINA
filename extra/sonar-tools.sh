#!/bin/bash

cd build
make clean
make 2>&1 | tee make-sonar.log 

ctest -D Experimental

cd ..

cppcheck -j8 --enable=all \
	-I build \
	-I build/Plugins/AppositionSurface \
	-I build/Plugins/CountingFrame/ \
	-I build/Plugins/SegmhaImporter \
	--xml . 2> build/cppcheck-sonar.xml

gcovr -x -r . > build/coverage-sonar.xml

rats --xml api > build/rats-sonar.xml

sonar-runner
