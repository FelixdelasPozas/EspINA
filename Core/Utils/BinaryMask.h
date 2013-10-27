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

#include <itkImage.h>
#include <itkSmartPointer.h>

namespace EspINA
{
  template<typename T > class BinaryMask
  {
    public:
      using PixelType    = T;
      using IndexType    = struct index { int x; int y; int z; index(): x(0), y(0), z(0) {} };
      using Spacing      = struct spacing { int x; int y; int z; spacing(): x(0), y(0), z(0) {} };
      using itkImageType = itk::Image<T,3>;

      //- BINARY MASK CLASS  ----------------------------------------------------------------
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

      friend class const_iterator;
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
      //- CONST ITERATOR CLASS --------------------------------------------------------------
      class const_iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
          using pointer = BinaryMask<T>*;

          const_iterator(pointer mask)
          : m_mask(mask), m_pos(0), m_bitPos(0)
          {
          }

          bool operator==(const const_iterator& other) const
          {
            return ((m_mask == other.m_mask) && (m_pos == other.m_pos) && (m_bitPos == other.m_bitPos));
          }

          bool operator!=(const const_iterator& other) const
          {
            return ((m_mask != other.m_mask) || (m_pos != other.m_pos) || (m_bitPos == other.m_bitPos));
          }

          const T& operator*() const
          {
            if ((m_mask->m_image[m_pos] & (1 << m_bitPos)) == (1 << m_bitPos))
              return m_mask->m_foregroundValue;

            return m_mask->m_backgroundValue;
          }

          const_iterator& operator--()
          {
            if (m_pos == 0 && m_bitPos == 0)
              throw;

            if (m_bitPos == 0)
            {
              m_bitPos = 7;
              --m_pos;
            }
            else
              --m_bitPos;

            return *this;
          }

          const_iterator &operator++()
          {
            unsigned long long limit = m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2];

            if ((m_pos == limit - 1) && (m_bitPos == (limit % m_mask->m_integerSize)))
              throw;

            if (m_bitPos == 7)
            {
              m_bitPos = 0;
              ++m_pos;
            }
            else
              ++m_bitPos;

            return *this;
          }

        private:
          pointer            m_mask;
          unsigned long long m_pos;
          int                m_bitPos;
      };

      //- CONST REGION ITERATOR CLASS -------------------------------------------------------
      class const_region_iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
          using pointer = BinaryMask<T>*;

          const_region_iterator(pointer mask, const Bounds bounds)
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

          bool operator==(const const_region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask == other.m_mask);
            retValue &= ((m_bounds[0] == other.m_bounds[0]) && (m_bounds[1] == other.m_bounds[1]) && (m_bounds[2] == other.m_bounds[2]) &&
                         (m_bounds[3] == other.m_bounds[3]) && (m_bounds[4] == other.m_bounds[4]) && (m_bounds[5] == other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) = 0);
            retValue &= ((m_index.x == other.m_index.x) && (m_index.y == other.m_index.y) && (m_index.z == other.m_index.z));

            return retValue;
          }

          bool operator!=(const const_region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask != other.m_mask);
            retValue &= ((m_bounds[0] != other.m_bounds[0]) && (m_bounds[1] != other.m_bounds[1]) && (m_bounds[2] != other.m_bounds[2]) &&
                         (m_bounds[3] != other.m_bounds[3]) && (m_bounds[4] != other.m_bounds[4]) && (m_bounds[5] != other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) != 0);
            retValue &= ((m_index.x != other.m_index.x) && (m_index.y != other.m_index.y) && (m_index.z != other.m_index.z));

            return retValue;
          }

          const T& operator*() const
          {
            return m_mask->pixel(m_index);
          }

          const_region_iterator& operator--()
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

          const_region_iterator &operator++()
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

          friend class region_const_iterator;

        private:
          pointer            m_mask;
          Bounds             m_bounds;
          unsigned long long m_extent[6];
          IndexType          m_index;
      };

  };

} // namespace EspINA

//#include "Core/Utils/BinaryMask.cpp"

#endif // ESPINA_BINARY_MASK_H
