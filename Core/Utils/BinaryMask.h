/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ESPINA_BINARY_MASK_H
#define ESPINA_BINARY_MASK_H

#include "Core/Utils/Bounds.h"
#include <Core/EspinaTypes.h>

#include <itkImage.h>
#include <itkSmartPointer.h>

#include <QDebug>

namespace EspINA
{
  template<typename T > class BinaryMask
  {
    public:
      using PixelType    = T;
      using IndexType    = struct index { int x; int y; int z; index(): x(0), y(0), z(0) {} };
      using Spacing      = struct spacing { Nm x; Nm y; Nm z; spacing(): x(1), y(1), z(1) {} };
      using itkImageType = itk::Image<T,3>;

      //- BINARY MASK CLASS  ----------------------------------------------------------------
      explicit BinaryMask(const Bounds& bounds, const Spacing spacing = Spacing());
      virtual ~BinaryMask() {};

      Bounds bounds() const                            { return m_bounds; }

      PixelType backgroundValue() const                { return m_backgroundValue; }
      void setBackgroundValue(const T backgroundValue) { m_backgroundValue = backgroundValue; }

      PixelType foregroundValue() const                { return m_foregroundValue; }
      void setForegroundValue(const T foregroundValue) { m_foregroundValue = foregroundValue; }

      unsigned long long numberOfVoxels()              { return m_size[0] * m_size[1] * m_size[2]; }

      inline void setPixel(const IndexType& index);
      inline void unsetPixel(const IndexType& index);
      inline PixelType pixel(const IndexType& index) const;

      typename itkImageType::Pointer itkImage() const;

      friend class iterator;
      friend class region_const_iterator;

    private:
      Bounds        m_bounds;
      PixelType     m_backgroundValue;
      PixelType     m_foregroundValue;
      int          *m_image;
      int           m_integerSize;
      Spacing       m_spacing;
      unsigned long m_size[3];
      IndexType     m_origin;

    public:
      //- ITERATOR CLASS --------------------------------------------------------------------
      class iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          iterator(BinaryMask<T> *mask)
          : m_mask(mask), m_pos(0), m_bitPos(0)
          {
          }

          virtual ~iterator() {};

          iterator begin()
          {
            return iterator(*this);
          }

          iterator end()
          {
            unsigned long long pos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize) + 1;

            return iterator(*this, pos);
          }

          void goToBegin()
          {
            m_pos = 0;
            m_bitPos = 0;
          }

          void goToEnd()
          {
            m_pos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize) + 1;
            m_bitPos = 0;
          }

          bool isAtEnd() const
          {
            return (m_pos == ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize) + 1);
          }

          bool operator==(const iterator& other) const
          {

            return ((m_mask == other.m_mask) && (m_pos == other.m_pos) && (m_bitPos == other.m_bitPos));
          }

          bool operator!=(const iterator& other) const
          {
            return ((m_mask != other.m_mask) || (m_pos != other.m_pos) || (m_bitPos == other.m_bitPos));
          }

          const T& Get() const
          {
            if (isAtEnd())
              throw;

            if ((m_mask->m_image[m_pos] & (1 << m_bitPos)) == (1 << m_bitPos))
              return m_mask->m_foregroundValue;

            return m_mask->m_backgroundValue;
          }

          void Set()
          {
            m_mask->m_image[m_pos] = m_mask->m_image[m_pos] | (1 << m_bitPos);
          }

          iterator& operator--()
          {
            if (m_pos == 0 && m_bitPos == 0)
              throw;

            if (isAtEnd())
            {
              m_pos = (m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize;
              m_bitPos = (m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize;
              return *this;
            }

            if (m_bitPos == 0)
            {
              m_bitPos = m_mask->m_integerSize - 1;
              --m_pos;
            }
            else
              --m_bitPos;

            return *this;
          }

          iterator &operator++()
          {
            unsigned long long limit = (m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize;
            int bitLimit = (m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize;

            if (isAtEnd())
              throw;

            if ((m_pos == limit) && (m_bitPos == bitLimit))
            {
              m_pos = limit + 1;
              m_bitPos = 0;
            }

            if (m_bitPos == m_mask->m_integerSize-1)
            {
              m_bitPos = 0;
              ++m_pos;
            }
            else
              ++m_bitPos;

            return *this;
          }

        protected:
          iterator(const iterator& it, const unsigned long long pos = 0, const int bitPos = 0)
          : m_mask(it.m_mask)
          , m_pos(pos)
          , m_bitPos(bitPos)
          {
          }

          BinaryMask<T>     *m_mask;
          unsigned long long m_pos;
          int                m_bitPos;
      };

      //- CONST ITERATOR CLASS --------------------------------------------------------------
      class const_iterator
      : public iterator
      {
        public:
          const_iterator(BinaryMask<T>* mask)
          : iterator(mask)
          {
          };

          virtual ~const_iterator() {};

        private:
          void Set();
      };

      //- REGION ITERATOR CLASS -------------------------------------------------------------
      class region_iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          region_iterator(BinaryMask<T> *mask, const Bounds &bounds)
          : m_mask(mask), m_bounds(bounds)
          {
            m_extent[0] = static_cast<int>(m_bounds[0]/m_mask->m_spacing.x);
            m_extent[1] = static_cast<int>(m_bounds[1]/m_mask->m_spacing.x);
            m_extent[2] = static_cast<int>(m_bounds[2]/m_mask->m_spacing.y);
            m_extent[3] = static_cast<int>(m_bounds[3]/m_mask->m_spacing.y);
            m_extent[4] = static_cast<int>(m_bounds[4]/m_mask->m_spacing.z);
            m_extent[5] = static_cast<int>(m_bounds[5]/m_mask->m_spacing.z);

            m_index.x = m_extent[0];
            m_index.x = m_extent[2];
            m_index.x = m_extent[4];
          }

          virtual ~region_iterator() {};

          region_iterator begin()
          {
            IndexType index;
            index.x = m_extent[0];
            index.y = m_extent[2];
            index.z = m_extent[4];

            return region_iterator(*this, index);
          }

          region_iterator end()
          {
            IndexType index;
            index.x = m_extent[1] + 1;
            index.y = m_extent[3] + 1;
            index.z = m_extent[5] + 1;

            return region_iterator(*this, index);
          }

          void goToBegin()
          {
            m_index.x = m_extent[0];
            m_index.y = m_extent[2];
            m_index.z = m_extent[4];
          }

          void goToEnd()
          {
            m_index.x = m_extent[1] + 1;
            m_index.y = m_extent[3] + 1;
            m_index.z = m_extent[5] + 1;
          }

          bool isAtEnd() const
          {
            return ((m_index.x == m_extent[1] + 1) && (m_index.x == m_extent[3] + 1) && (m_index.x == m_extent[5] + 1));
          }

          bool operator==(const region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask == other.m_mask);
            retValue &= ((m_bounds[0] == other.m_bounds[0]) && (m_bounds[1] == other.m_bounds[1]) && (m_bounds[2] == other.m_bounds[2]) &&
                         (m_bounds[3] == other.m_bounds[3]) && (m_bounds[4] == other.m_bounds[4]) && (m_bounds[5] == other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) == 0);
            retValue &= ((m_index.x == other.m_index.x) && (m_index.y == other.m_index.y) && (m_index.z == other.m_index.z));

            return retValue;
          }

          bool operator!=(const region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask != other.m_mask);
            retValue &= ((m_bounds[0] != other.m_bounds[0]) && (m_bounds[1] != other.m_bounds[1]) && (m_bounds[2] != other.m_bounds[2]) &&
                         (m_bounds[3] != other.m_bounds[3]) && (m_bounds[4] != other.m_bounds[4]) && (m_bounds[5] != other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) != 0);
            retValue &= ((m_index.x != other.m_index.x) && (m_index.y != other.m_index.y) && (m_index.z != other.m_index.z));

            return retValue;
          }

          const T& Get() const
          {
            // NOTE: we need this to avoid returning a temporary value
            m_getReturnValue = m_mask->pixel(m_index);

            return m_getReturnValue;
          }

          region_iterator& operator--()
          {
            if ((m_index.x == m_extent[0]) && (m_index.y == m_extent[2]) && (m_index.z == m_extent[4]))
              throw;

            if (m_index.x == m_extent[0])
            {
              m_index.x = m_extent[1];
              if (m_index.y == m_extent[2])
              {
                m_index.y = m_extent[3];
                --m_index.z;
              }
              else
                --m_index.y;
            }
            else
              --m_index.x;

            return *this;
          }

          region_iterator &operator++()
          {
            if ((m_index.x == m_extent[1]) && (m_index.y == m_extent[3]) && (m_index.z == m_extent[5]))
              throw;

            if (m_index.x == m_extent[1])
            {
              m_index.x = m_extent[0];
              if (m_index.y == m_extent[3])
              {
                m_index.y = m_extent[2];
                ++m_index.z;
              }
              else
                ++m_index.y;
            }
            else
              ++m_index.x;

            return *this;
          }

        protected:
          region_iterator(const region_iterator &it, const IndexType &index)
          : m_mask(it.m_mask)
          , m_bounds(it.m_bounds)
          {
            memcpy(m_extent, it.m_extent, 6*sizeof(unsigned long long));

            m_index.x = index.x;
            m_index.y = index.y;
            m_index.z = index.z;
          }

          BinaryMask<T>     *m_mask;
          Bounds             m_bounds;
          unsigned long long m_extent[6];
          IndexType          m_index;
          mutable PixelType  m_getReturnValue; // just to return the value of Get();
      };

      //- CONST REGION ITERATOR CLASS -------------------------------------------------------
      class const_region_iterator
      : public region_iterator
      {
        public:
          const_region_iterator(BinaryMask<T> *mask, const Bounds &bounds)
          : region_iterator(mask, bounds)
          {
          }

          virtual ~const_region_iterator() {};
        private:
          void Set();
      };

  };

} // namespace EspINA

#endif // ESPINA_BINARY_MASK_H
