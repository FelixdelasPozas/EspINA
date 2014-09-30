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

## How do I get it?

## Source code compilation

# Compile quazip

Ubuntu dependencies:
libqt4-dev
zlib1g-dev

First download Quazip from its SourceForge page and untar it

```sh
tar xvzf quazip-0.7.tar.gz
cd quazip-0.7
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_QT4=ON ..
```

# Compile VTK 6
Ubuntu dependencies:
freeglut3-dev
libxt-dev
libqtwebkit-dev
libqt4-opengl-dev
libboost-graph-dev
libboost-regex-dev

```sh
tar xzvf VTK-6.1.0
cd VTK-6.1.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DModule_vtkInfovisBoost=ON -DModule_vtkInfovisBoostGraphAlg=ON -DVTK_Group_Qt=ON ..

```

# Compile ITK 4.4.2
Ubuntu dependencies:
freeglut3-dev
libxt-dev
libqtwebkit-dev
libqt4-opengl-dev
libboost-graph-dev

```sh
tar xzvf InsightToolkit-4.4.2.tar.gz
cd InsightToolkit-4.4.2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DModule_ITKVtkGlue=ON -DVTK_DIR=<path to vtk build directory>  ..
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
mkdir build
cd build
ccmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11 ..
```
