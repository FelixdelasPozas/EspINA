ESPINA
=======

An interactive neuron analyzer.

## Introduction

EspINA is a user-friendly tool which performs segmentation and analysis of elements in a reconstructed 3D volume of the brain, and greatly facilitates and accelerates these processes. It allows visualization and segmentation of large image stacks datasets, both from electron microscopy (e.g. FIB/SEM) and confocal laser microscopy.

You can find a much more detailed description of EspINA in its [official webpage](https://cajalbbp.es/espina/). You can see some videos of EspINA [here](https://www.youtube.com/channel/UCN3kLTMxaJXkEJrvsJbx-ww).

If you have used EspINA to get your results and you have published them (congratulations!!) please, let us know and your publication will also appear [here](https://cajalbbp.es/espina/#publications).

## Dependencies and compilation

EspINA uses several external libraries, namely:

- [zlib](https://zlib.net/)
- [Quazip](https://github.com/stachenov/quazip)
- [Visualization Toolkit, VTK](https://vtk.org/download/)
- [Insight Toolkit, ITK](https://itk.org/)
- [Boost](https://www.boost.org/)
- [Qt 5 Framework](https://www.qt.io/)

To compile EspINA you'll need:

- cross-platform build system: [CMake](http://www.cmake.org/cmake/resources/software.html).
- C++ compiler: [Mingw64](http://sourceforge.net/projects/mingw-w64/) on Windows or [GCC](https://gcc.gnu.org/) on Linux systems.

## Install

Download and install the latest build from the [releases](https://github.com/FelixdelasPozas/EspINA/releases) page. 

## Acknowledgments

EspINA is and open source software developed by the [Universidad Politécnica de Madrid](https://www.upm.es/) and [Universidad Rey Juan Carlos](https://www.urjc.es) with the collaboration of [CSIC](https://www.csic.es/), under the [Cajal Blue Brain Project](https://cajalbbp.es). 

# Repository information

**Version**: 2.9.15

**Status**: finished

**cloc statistics**

| Language                     |files          |blank        |comment           |code      |
|:-----------------------------|--------------:|------------:|-----------------:|---------:|
| C++                          | 783           | 27558       | 23925            | 101885   |
| C/C++ Header                 | 513           | 12624       | 29154            |  25153   |
| CMake                        |  66           |   441       |   213            |   2814   |
| **Total**                    | **1362**      | **40623**   | **53292**        | **129852** |
