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
- Qt4
- Metadona (optional)

## Compilation dependencies
cmake
# (ccmake)
cmake-curses-gui
## How do I get it?
wget http://cajalbbp.cesvima.upm.es/SW/espina-2.0.0.tar.gz

## Source code compilation

# XLSLIB
wget http://sourceforge.net/projects/xlslib/files/xlslib-package-2.5.0.zip/download
mv download xlslib-package-2.5.0.zip
dtrx xlslib-package-2.5.0.zip
cd xlslib/xlslib
./configure
make
make check

# Compile quazip

Ubuntu dependencies:
libqt4-dev
zlib1g-dev

First download Quazip from its SourceForge page and untar it

```sh
wget http://sourceforge.net/projects/quazip/files/quazip/0.7/quazip-0.7.tar.gz/download
mv download quazip-0.7
tar xvzf quazip-0.7.tar.gz
cd quazip-0.7
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_QT4=ON ..
sudo make install
sudo make clean
```

# Compile VTK 6
Ubuntu dependencies:
freeglut3-dev
libxt-dev
libqtwebkit-dev
libqt4-opengl-dev
libboost-graph-dev

```sh
wget http://www.vtk.org/files/release/6.1/VTK-6.1.0.tar.gz
tar xzvf VTK-6.1.0
cd VTK-6.1.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DModule_vtkInfovisBoost=ON -DModule_vtkInfovisBoostGraphAlgortihms=ON -DVTK_Group_Qt=ON ..
# On debian (but some ubuntu doc indicates the same)
# had to edit /usr/include/GL/glext.h
# to include lines:
# typedef ptrdiff_t GLsizeiptr;
# typedef ptrdiff_t GLintptr; 
# on lines 480, 481 before
# typedef void ( *PFNGLXCOPYBUFFERSUBDATANVPROC) (Display *dpy, GLXContext readCtx, GLXContext writeCtx, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
make -j8
make check
```

# Compile ITK 4.4.2
Ubuntu dependencies:
freeglut3-dev
libxt-dev
libqtwebkit-dev
libqt4-opengl-dev
libboost-graph-dev

```sh
wget http://sourceforge.net/projects/itk/files/itk/4.4/InsightToolkit-4.4.2.tar.gz/download
mv download InsightToolkit-4.4.2.tar.gz
tar xzvf InsightToolkit-4.4.2.tar.gz
cd InsightToolkit-4.4.2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DModule_ITKVtkGlue=ON -Wno-dev -DVTK_DIR=<path to vtk build directory>  ..
make -j8
make check
```

# Compile Metadona 1.0.0 (Optional)
Ubuntu dependencies:

```sh
tar xzvf metadona.tar.gz
cd metadona
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 ..
```

# Compile ESPINA
Ubuntu dependencies:
libboost-regex-dev

```sh
wget http://cajalbbp.cesvima.upm.es/SW/espina-2.0.0.tar.gz
tar xzvf espina-2.0.0.tar.gz
cd espina
mkdir build
cd build
ccmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 ..
# EMPTY cache
# Press c to configure
# It will fail if it doens't find the previous build directories
# press e
# To give quazip link
#	select line with arrows
#	press enter to edit option
#	modify line
#	press c
#Repeat for other libraries
# VTK_DIR=~/Downloads/VTK-6.1.0/build
# ITK_DIR=~/Downloads/InsightToolkit-4.4.2/build
# XLSLIB_INCLUDE_DIR=xlslib/src
# XLSLIB_LIBRARY=xlslib/src/.libs/libxls.so
#Turn autoversion off
#press g to generate

# N = n+1 cores to compile
make -jN
```
