/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_BINARY_MASK_H
#define ESPINA_BINARY_MASK_H

#include "Core/Utils/Bounds.h"

#include "itkImage.h"
#include <itkSmartPointer.h>

namespace EspINA
{
  //- Binary Mask class -----------------------------------------------------------------
  template<typename T> class BinaryMask
  {
    using PixelType    = T;
    using IndexType    = struct index { int x; int y; int z; index(): x(0), y(0), z(0) {} };
    using Spacing      = struct spacing { int x; int y; int z; spacing(): x(0), y(0), z(0) {} };
    using itkImageType = itk::Image<T,3>;

    class Iterator;
    class ConstIterator;
    class RegionIterator;
    class RegionConstIterator;

    public:
      explicit BinaryMask(const Bounds& bounds, const Spacing spacing);
      virtual ~BinaryMask();

      Bounds bounds() const                      { return m_bounds; }

      PixelType backgroundValue() const          { return m_backgroundValue; }
      void setBackgroundValue(T backgroundValue) { m_backgroundValue = backgroundValue; }

      PixelType foregroundValue() const          { return m_foregroundValue; }
      void setForegroundValue(T foregroundValue) { m_foregroundValue = foregroundValue; }

      inline void setPixel(IndexType& index);
      inline void unsetPixel(IndexType& index);
      inline PixelType pixel(IndexType& index) const;

      typename itkImageType::Pointer itkImage() const;

    private:
      Bounds        m_bounds;
      PixelType     m_backgroundValue;
      PixelType     m_foregroundValue;
      int          *m_image;
      int           m_integerSize;
      Spacing       m_spacing;
      unsigned long m_size[3];
      IndexType     m_origin;

      friend class Iterator;
      friend class ConstIterator;
      friend class RegionIterator;
      friend class RegionConstIterator;
  };

  //- Binary Mask class iterator --------------------------------------------------------
  template<typename T> class BinaryMask<T>::Iterator
  {
    public:
      Iterator(BinaryMask<T> image, Bounds bounds);
      ~Iterator();

      T get() const;
      void set();

      Iterator& operator++();
      Iterator& operator--();
  };

  //- Binary Mask class const iterator --------------------------------------------------
  template<typename T> class BinaryMask<T>::ConstIterator
  {
    public:
      ConstIterator(BinaryMask<T> image, Bounds bounds);
      ~ConstIterator();

      const T get() const;

      ConstIterator& operator++();
      ConstIterator& operator--();
  };

  //- Binary Mask region iterator--------------------------------------------------------
  template<typename T> class BinaryMask<T>::RegionIterator
  {
    public:
      RegionIterator(BinaryMask<T> image, Bounds bounds);
      ~RegionIterator();

      T get() const;
      void set();

      RegionIterator& operator++();
      RegionIterator& operator--();
  };

  //- Binary Mask region const interator ------------------------------------------------
  template<typename T> class BinaryMask<T>::RegionConstIterator
  {
    public:
      RegionConstIterator(BinaryMask<T> image, Bounds bounds);
      ~RegionConstIterator();

      const T get() const;

      RegionConstIterator& operator++();
      RegionConstIterator& operator--();
  };

}

#endif // ESPINA_BINARY_MASK_H
