/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

#ifndef CORE_UTILS_HISTOGRAM_H_
#define CORE_UTILS_HISTOGRAM_H_

#include <Core/EspinaCore_Export.h>

// ESPINA
#include <Core/Types.h>

// C++
#include <numeric>
#include <vector>

// Qt
#include <QDebug>

namespace ESPINA
{
  namespace Core
  {
    namespace Utils
    {
      /** \class Histogram
       * \brief Maintains a histogram for images with unsigned char pixel values. Most of the values are
       * only valid after an update() method to avoid constant computations when adding values to the
       * histogram.
       *
       */
      class EspinaCore_EXPORT Histogram
      {
        public:
          /** \brief Histogram class constructor.
           *
           */
          explicit Histogram();

          /** \brief Histogram class virtual destructor.
           *
           */
          virtual ~Histogram()
          {};

          /** \brief Updates the histogram median, mode, major and minor values.
           *
           */
          void update();

          /** \brief Returns true if the histogram is empty and false otherwise.
           *
           */
          const bool isEmpty() const;

          /** \brief Helper method that adds the values of the stack to the histogram where the
           * voxels with the same index have a value of SEG_VOXEL_VALUE in the given image.
           *
           */
          void addValues(itkVolumeType::Pointer image, itkVolumeType::Pointer stack);

          /** \brief Helper method that adds the values of the image region to the histogram.
           *
           */
          void addValues(itkVolumeType::Pointer image, const itkVolumeType::RegionType &region);

          /** \brief Helper method that adds the values of the given buffer to the histogram.
           * \param[in] buffer Unsigned char buffer.
           * \param[in] length Number of values to read from the buffer.
           *
           */
          void addValues(unsigned char *buffer, const unsigned long length);

          /** \brief Adds a value to the histogram.
           * \param[in] value Value to add.
           *
           */
          inline void addValue(const unsigned char value)
          {
            m_values[value]++;
          }

          /** \brief Returns the number of values of the histogram. Valid only after update().
           *
           */
          inline const unsigned long long count() const
          {
            return m_count;
          }

          /** \brief Returns the probability of the given value. Valid only after update();
           * \param[in] value Unsigned char value.
           */
          inline const double probability(const unsigned char value) const
          {
            return m_values[value]/static_cast<double>(m_count);
          }

          /** \brief Returns the number of voxels with the given value.
           * \param[in] value Unsigned char value.
           *
           */
          inline const unsigned long long values(const unsigned char value) const
          {
            return m_values[value];
          }

          /** \brief Returns the minor value. Valid only after update().
           *
           */
          inline const unsigned char minorValue() const
          {
            return m_minor;
          }

          /** \brief Returns the major value. Valid only after update().
           *
           */
          inline const unsigned char majorValue() const
          {
            return m_major;
          }

          /** \brief Returns the mode value. Valid only after update().
           *
           */
          inline const unsigned char modeValue() const
          {
            return m_mode;
          }

          /** \brief Returns the median value. Valid only after update().
           *
           */
          inline const unsigned char medianValue() const
          {
            return m_median;
          }

          /** \brief Resets the internal values (empties the histogram).
           *
           */
          void reset();

          /** \brief Returns the threshold value from the median value that contains "percent" of the values.
           * \param[in] percent Value in (0-1].
           *
           */
          const unsigned char threshold(const float percent) const;
  
          /** \brief Operator+ for histograms.
           * \param[in] other Histogram object reference.
           *
           */
          Histogram &operator+(const Histogram &other);

          /** \brief Operator= for histograms.
           * \param[in] other Histogram object reference.
           *
           */
          Histogram &operator=(const Histogram &other);

        private:
          std::vector<unsigned long long> m_values; /** values count            */
          unsigned char                   m_major;  /** major value.            */
          unsigned char                   m_minor;  /** minor value.            */
          unsigned char                   m_median; /** median value.           */
          unsigned char                   m_mode;   /** mode value.             */
          unsigned long long              m_count;  /** total number of values. */
      };
    } // namespace Utils
  } // namespace Core
} // namespace ESPINA

/** \brief Operator<< for QDebug streams. Only gives valid values if the histogram has been updated.
 * \param[inout] stream QDebug stream.
 * \param[in] histogram Histogram object reference.
 *
 */
QDebug operator<< (QDebug stream, const ESPINA::Core::Utils::Histogram& histogram);

#endif // CORE_UTILS_HISTOGRAM_H_
