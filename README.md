ESPINA
=======

An interactive neuron analyzer.

## Introduction

## Dependencies

EspINA relies in several external libraries, namely:

- zlib (or its variants)
- Quazip
- VTK
- ITK
- Boost
- Qt5
- Metadona (optional)

## Compilation dependencies
### cmake
```sh
sudo apt-get -y install cmake
```
### ccmake
```sh
sudo apt-get -y install cmake-curses-gui
```
### rest of dependencies (bundle)
```sh
sudo apt-get -y install g++ gcc autoconf automake cmake-curses-gui libqt4-dev zlib1g-dev freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev libboost-regex-dev #g++-4.4
```

## How do I get it?
```sh
wget http://cajalbbp.cesvima.upm.es/SW/espina-2.0.0.tar.gz
```

## Source code compilation

#-> Compile XLSLIB
```sh
wget http://sourceforge.net/projects/xlslib/files/xlslib-package-2.5.0.zip/download
mv download xlslib-package-2.5.0.zip
unzip xlslib-package-2.5.0.zip
cd xlslib/xlslib
./configure
make
cd ../..
```

#-> Compile quazip
#Ubuntu dependencies (included in bundle): libqt4-dev / zlib1g-dev
```sh
sudo apt-get -y install libqt4-dev zlib1g-dev
```
###Download Quazip from its SourceForge, untar, compile and install it
```sh
wget http://sourceforge.net/projects/quazip/files/quazip/0.7/quazip-0.7.tar.gz/download
mv download quazip-0.7.tar.gz
tar xvzf quazip-0.7.tar.gz
cd quazip-0.7
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_QT4=ON ..
sudo make install
sudo make clean
cd ../..
```

#-> Compile VTK 6
###Ubuntu dependencies (included in bundle): freeglut3-dev / libxt-dev / libqtwebkit-dev / libqt4-opengl-dev / libboost-graph-dev
```sh
sudo apt-get -y install freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev
```
###Download VTK 6 from its website, untar, compile and build it
```sh
wget http://www.vtk.org/files/release/6.1/VTK-6.1.0.tar.gz
tar xzvf VTK-6.1.0
cd VTK-6.1.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DModule_vtkInfovisBoost=ON -DModule_vtkInfovisBoostGraphAlgorithms=ON -DVTK_Group_Qt=ON ..
# On debian (but some ubuntu doc indicates the same)
# had to edit /usr/include/GL/glext.h
# to include lines:
# typedef ptrdiff_t GLsizeiptr;
# typedef ptrdiff_t GLintptr; 
# on lines 480, 481 before
# typedef void ( *PFNGLXCOPYBUFFERSUBDATANVPROC) (Display *dpy, GLXContext readCtx, GLXContext writeCtx, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
make -j8
cd ../..
```

#-> Compile ITK 4.4.2
###Ubuntu dependencies (included in bundle): freeglut3-dev / libxt-dev / libqtwebkit-dev / libqt4-opengl-dev / libboost-graph-dev
```sh
sudo apt-get -y install freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev
```
###Download ITK 4.4.2 from its sourceforge, untar, compile and build it
```sh
wget http://sourceforge.net/projects/itk/files/itk/4.4/InsightToolkit-4.4.2.tar.gz/download
mv download InsightToolkit-4.4.2.tar.gz
tar xzvf InsightToolkit-4.4.2.tar.gz
cd InsightToolkit-4.4.2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DModule_ITKVtkGlue=ON -Wno-dev -DModule_ITKReview=ON -DVTK_DIR=<path to vtk build directory> ..
make -jN
cd ../..
```

#-> Compile Metadona 1.0.0 (Optional)
```sh
tar xzvf metadona.tar.gz
cd metadona
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 ..
```

#-> Compile ESPINA
###Ubuntu dependencies (included in bundle): libboost-regex-dev
```sh
sudo apt-get -y install libboost-regex-dev
```
###Download ESPINA from its repository, untar, compile and build it
```sh
wget http://cajalbbp.cesvima.upm.es/SW/espina-2.0.0.tar.gz
tar xzvf espina-2.0.0.tar.gz
mv espina-developers* source
mkdir build
cd build
cmake ../source -DCMAKE_BUILD_TYPE=Debug -DAUTOMATIC_VERSION=OFF -DBUILD_TESTING=OFF -DBUILD_UNIT_TESTS=OFF -DITK_DIR=$PWD/../InsightToolkit-4.4.2/build -DVTK_DIR=$PWD/../VTK-6.1.0/build -DXLSLIB_INCLUDE_DIR=$PWD/../xlslib/xlslib/src -DXLSLIB_LIBRARY=$PWD/../xlslib/xlslib/src/.libs/libxls.so.3 -DCMAKE_CXX_FLAGS=-std=c++11
make -jN   # where N = n+1 cores to compile
```



#-> Script for Ubuntu (tested in Ubuntu-14.04.3)
###Remember to change the make -j3 option depending on the number of cores that you have
```sh
#!/bin/bash

cd ~
mkdir espinaDK
cd espinaDK

#-> Download cmake
sudo apt-get -y install cmake

#-> Ubuntu dependencies:
sudo apt-get -y install g++ gcc autoconf automake cmake-curses-gui libqt4-dev zlib1g-dev freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev libboost-regex-dev

#-> Compile XLSLIB
wget http://sourceforge.net/projects/xlslib/files/xlslib-package-2.5.0.zip/download
mv download xlslib-package-2.5.0.zip
unzip xlslib-package-2.5.0.zip
cd xlslib/xlslib
#export CC=gcc-4.4 CXX=g++-4.4
./configure
make
cd ../..


#-> Compile quazip
#Ubuntu dependencies: libqt4-dev zlib1g-dev

wget http://sourceforge.net/projects/quazip/files/quazip/0.7/quazip-0.7.tar.gz/download
mv download quazip-0.7.tar.gz
tar xvzf quazip-0.7.tar.gz
cd quazip-0.7
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_QT4=ON ..
sudo make install
sudo make clean
cd ../..


#-> Compile VTK 6
#Ubuntu dependencies: freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev

wget http://www.vtk.org/files/release/6.1/VTK-6.1.0.tar.gz
tar xzvf VTK-6.1.0.tar.gz
cd VTK-6.1.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DModule_vtkInfovisBoost=ON -DModule_vtkInfovisBoostGraphAlgorithms=ON -DVTK_Group_Qt=ON ..
make -j3
cd ../..


#-> Compile ITK 4.4.2
#Ubuntu dependencies: freeglut3-dev libxt-dev libqtwebkit-dev libqt4-opengl-dev libboost-graph-dev

wget http://sourceforge.net/projects/itk/files/itk/4.4/InsightToolkit-4.4.2.tar.gz/download
mv download InsightToolkit-4.4.2.tar.gz
tar xzvf InsightToolkit-4.4.2.tar.gz
cd InsightToolkit-4.4.2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DModule_ITKVtkGlue=ON -Wno-dev -DModule_ITKReview=ON -DVTK_DIR=$PWD/../../VTK-6.1.0/build ..
make -j3
cd ../..


#-> Compile Metadona 1.0.0 (Optional)
#tar xzvf metadona.tar.gz
#cd metadona
#mkdir build
#cd build
#cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 ..


#-> Compile ESPINA
#Ubuntu dependencies: libboost-regex-dev

wget http://cajalbbp.cesvima.upm.es/SW/espina-2.0.0.tar.gz
tar xzvf espina-2.0.0.tar.gz
mv espina-developers* source
mkdir build
cd build
cmake ../source -DCMAKE_BUILD_TYPE=Debug -DAUTOMATIC_VERSION=OFF -DBUILD_TESTING=OFF -DBUILD_UNIT_TESTS=OFF -DITK_DIR=$PWD/../InsightToolkit-4.4.2/build -DVTK_DIR=$PWD/../VTK-6.1.0/build -DXLSLIB_INCLUDE_DIR=$PWD/../xlslib/xlslib/src -DXLSLIB_LIBRARY=$PWD/../xlslib/xlslib/src/.libs/libxls.so.3 -DCMAKE_CXX_FLAGS=-std=c++11
make -j3
cd ../..
```
