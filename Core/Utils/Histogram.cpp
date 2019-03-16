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

// ESPINA
#include <Core/Utils/Histogram.h>
#include <Core/Utils/EspinaException.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// C++
#include <limits>

// Qt
#include <QString>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

//--------------------------------------------------------------------
Histogram::Histogram()
{
  reset();
}

//--------------------------------------------------------------------
void Histogram::update()
{
  m_minor = std::numeric_limits<unsigned char>::max();
  m_major = std::numeric_limits<unsigned char>::min();
  m_count = 0;
  unsigned long value = 0;
  unsigned long max   = 0;

  for(unsigned short i = 0; i < 256; ++i)
  {
    if(m_values[i] != 0)
    {
      if(i < m_minor) m_minor = i;
      if(i > m_major) m_major = i;
      if(m_values[i] > max)
      {
        max = m_values[i];
        m_mode = i;
      }
      m_count += m_values[i];
      value += m_values[i] * i;
    }
  }

  if(m_count != 0) m_median = value / m_count;
}

//--------------------------------------------------------------------
void Histogram::addValues(itkVolumeType::Pointer image, itkVolumeType::Pointer stack)
{
  if(!stack->GetLargestPossibleRegion().IsInside(image->GetLargestPossibleRegion()))
  {
    QString message{"Given image is not completely inside the stack."};
    QString details{"Histogram::addValues(image, region) -> "};

    throw Utils::EspinaException(message, details);
  }

  const auto region = image->GetLargestPossibleRegion();
  itk::ImageRegionConstIteratorWithIndex<itkVolumeType> sIt(stack, region);
  sIt.GoToBegin();
  itk::ImageRegionConstIteratorWithIndex<itkVolumeType> rIt(image, region);
  rIt.GoToBegin();
  while(!rIt.IsAtEnd())
  {
    if(rIt.Value() == SEG_VOXEL_VALUE)
    {
      addValue(sIt.Value());
    }
    ++rIt;
    ++sIt;
  }
}

//--------------------------------------------------------------------
void Histogram::addValues(itkVolumeType::Pointer image, const itkVolumeType::RegionType& region)
{
  if(!image->GetLargestPossibleRegion().IsInside(region))
  {
    QString message{"Given region is not completely inside the image."};
    QString details{"Histogram::addValues(image, region) -> "};

    throw Utils::EspinaException(message, details);
  }

  itk::ImageRegionConstIteratorWithIndex<itkVolumeType> it(image, region);
  it.GoToBegin();
  while(!it.IsAtEnd())
  {
    addValue(it.Value());
    ++it;
  }
}

//--------------------------------------------------------------------
void Histogram::reset()
{
  m_values = std::vector<unsigned long long>(256,0);
  m_major  = std::numeric_limits<unsigned char>::min();
  m_minor  = std::numeric_limits<unsigned char>::max();
  m_median = 0;
  m_mode   = 0;
  m_count  = 0;
}

//--------------------------------------------------------------------
const unsigned char Histogram::threshold(const float percent) const
{
  if(m_count == 0 ||
    (m_major == std::numeric_limits<unsigned char>::min() &&
     m_minor == std::numeric_limits<unsigned char>::max()))
  {
    // not updated, invalid median value.
    return 0;
  }

  // invalid parameter
  if(percent <= 0 || percent > 1) return 0;

  unsigned char value = 0;
  for(unsigned int i = 1; i < 256; ++i)
  {
    unsigned long long sum = 0;
    for(int j = m_median - i; j <= static_cast<int>(m_median + i); ++j)
    {
      if(j < 0 || j > 255) continue;
      sum += m_values[j];
    }

    if((sum/static_cast<float>(m_count)) >= percent)
    {
      value = i;
      break;
    }
  }

  return value;
}

//--------------------------------------------------------------------
QDebug operator <<(QDebug stream, const ESPINA::Core::Utils::Histogram& histogram)
{
  if(histogram.majorValue() == std::numeric_limits<unsigned char>::min() &&
     histogram.minorValue() == std::numeric_limits<unsigned char>::max())
  {
    stream << "histogram not updated, update if first!";
  }
  else
  {
    const auto minor = histogram.minorValue();
    const auto major = histogram.majorValue();
    int count = 0;
    for(unsigned int i = 0; i < 256; ++i)
    {
      if(i < minor || i > major) continue;

      if(count != 0 && count % 8 == 0) stream << "\n";

      stream << QString("%1").arg(i, 3, 10, QChar(' ')) << "-" << QString("%1").arg(histogram.values(i), 6, 10, QChar(' ')) << "|";

      ++count;
    }
    stream << "\ntotal values" << histogram.count() << "\n";
    stream << "empty values" << (minor - 1) + (256 - major) << "\n";
    stream << "minor value" << minor << "\n";
    stream << "major value" << major << "\n";
    stream << "mode value" << histogram.modeValue() << "\n";
    stream << "median value" << histogram.medianValue() << "\n";

    stream << "90 % threshold" << histogram.threshold(0.9) << "\n";
    stream << "80 % threshold" << histogram.threshold(0.8) << "\n";
    stream << "70 % threshold" << histogram.threshold(0.7) << "\n";
    stream << "60 % threshold" << histogram.threshold(0.6) << "\n";
    stream << "50 % threshold" << histogram.threshold(0.5) << "\n";
  }

  return stream;
}

//--------------------------------------------------------------------
void Histogram::addValues(unsigned char* buffer, const unsigned long length)
{
  if(buffer && length > 0)
  {
    for(unsigned long i = 0; i < length; ++i)
    {
      addValue(buffer[i]);
    }
  }
}

//--------------------------------------------------------------------
Histogram& Histogram::operator +(const Histogram& other)
{
  for(int i = 0; i < 256; ++i) m_values[i] += other.values(i);
  update();

  return *this;
}

//--------------------------------------------------------------------
Histogram& Histogram::operator =(const Histogram& other)
{
  for(int i = 0; i < 256; ++i) m_values[i] = other.values(i);
  update();

  return *this;
}

//--------------------------------------------------------------------
const bool Histogram::isEmpty() const
{
  for(int i = 0; i < 256; ++i) if(m_values[i] != 0) return false;

  return true;
}
