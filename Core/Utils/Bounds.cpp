/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// ESPINA
#include "Bounds.h"

// C++
#include <tgmath.h>
#include <QStringList>

using namespace ESPINA;

//-----------------------------------------------------------------------------
Bounds::Bounds()
: m_bounds{0, -1, 0, -1, 0, -1}
{
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    m_lowerInclusion[idx(dir)] = true;
    m_upperInclusion[idx(dir)] = false;
  }
}

//-----------------------------------------------------------------------------
bool lowerBoundsInclusion(double value) {
  if (value == '[') return true;
  if (value == '(') return false;

  throw Invalid_Bounds_Token();
}

//-----------------------------------------------------------------------------
bool upperBoundsInclusion(double value) {
  if (value == ']') return true;
  if (value == ')') return false;

  throw Invalid_Bounds_Token();
}

//-----------------------------------------------------------------------------
Bounds::Bounds(std::initializer_list<double> bounds)
{
  int i = 0;
  switch (bounds.size()) {
    case 6: {
      for (auto b=bounds.begin(); b!=bounds.end(); ++b, ++i) {
        m_bounds[i] = *b;
      }

      for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
        m_lowerInclusion[idx(dir)] = true;
        m_upperInclusion[idx(dir)] = false;
      }
      break;
    }
    case 8: {
      auto b = bounds.begin();

      bool lowerInclusion = lowerBoundsInclusion(*b);
      ++b;

      for (; i < 6; ++b, ++i) {
        m_bounds[i] = *b;
      }
      bool upperInclusion = upperBoundsInclusion(*b);

      for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
        m_lowerInclusion[idx(dir)] = lowerInclusion;
        m_upperInclusion[idx(dir)] = upperInclusion;
      }
      break;
    }
    case 12: {
      auto b = bounds.begin();

      for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
        m_lowerInclusion[idx(dir)] = lowerBoundsInclusion(*b);
        ++b;
        m_bounds[i] = *b;
        i++; b++;
        m_bounds[i] = *b;
        i++; b++;
        m_upperInclusion[idx(dir)] = upperBoundsInclusion(*b);
        ++b;
      }
      break;
    }
    default:
      throw Wrong_Number_Initial_Values{};
  }

}

//-----------------------------------------------------------------------------
Bounds::Bounds(const NmVector3& point)
{
  for (int i = 0; i < 6; ++i)
  {
    m_bounds[i] = point[i/2];
  }
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    m_lowerInclusion[idx(dir)] = true;
    m_upperInclusion[idx(dir)] = true;
  }
}

//-----------------------------------------------------------------------------
Bounds::Bounds(Nm *bounds)
: Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]}
{
}

//-----------------------------------------------------------------------------
Bounds::Bounds(const QString& string)
{
  if (string.left(1)  != "{") throw Invalid_Bounds_Token();
  if (string.right(1) != "}") throw Invalid_Bounds_Token();

  QStringList ranges = string.split(",");

  if (ranges.size() != 6) throw Invalid_Bounds_Token();

  m_lowerInclusion[0] = ranges[0].startsWith("{[");
  m_lowerInclusion[1] = ranges[2].startsWith("[");
  m_lowerInclusion[2] = ranges[4].startsWith("[");
  m_upperInclusion[0] = ranges[1].endsWith("]");
  m_upperInclusion[1] = ranges[3].endsWith("]");
  m_upperInclusion[2] = ranges[5].endsWith("]}");

  m_bounds[0] = ranges[0].mid(2).toDouble();
  m_bounds[1] = ranges[1].mid(0, ranges[1].length()-1).toDouble();
  m_bounds[2] = ranges[2].mid(1).toDouble();
  m_bounds[3] = ranges[3].mid(0, ranges[3].length()-1).toDouble();
  m_bounds[4] = ranges[4].mid(1).toDouble();
  m_bounds[5] = ranges[5].mid(0, ranges[5].length()-2).toDouble();
}

//-----------------------------------------------------------------------------
QString Bounds::toString() const
{
  QString string = "{";
  int i = 0;
  for (auto axis : {Axis::X , Axis::Y, Axis::Z}) {
    if (i > 0) {string += ",";}
    string += areLowerIncluded(axis)?"[":"(";
    string += QString("%1,%2").arg(m_bounds[i]).arg(m_bounds[i+1]);
    string += areUpperIncluded(axis)?"]":")";
    i += 2;
  }
  string += "}";

  return string;
}

//-----------------------------------------------------------------------------
bool ESPINA::intersect(const Bounds& b1, const Bounds& b2, NmVector3 spacing)
{
//   auto lessThan         = [](double a, double b){return a <  b;};
  auto lessEqualThan    = [](double a, double b){return a <= b;};
//   auto greaterThan      = [](double a, double b){return a >  b;};
  auto greaterEqualThan = [](double a, double b){return a >= b;};

  bool overlap = true;
  int  i = 0;
  for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
    overlap &= lessEqualThan(b1[i], b2[i+1]) && greaterEqualThan(b1[i+1], b2[i]);

    if (areEqual(b1[i],   b2[i+1], spacing[i/2]))
    {
      bool b2UpperIncluded = b2.areUpperIncluded(dir) || (areEqual(b2[i], b2[i+1], spacing[i/2]) && b2.areLowerIncluded(dir));
      overlap &= b1.areLowerIncluded(dir) && b2UpperIncluded;
    }

    if (areEqual(b1[i+1], b2[i], spacing[i/2]))
    {
      bool b1UpperIncluded = b1.areUpperIncluded(dir) || (areEqual(b1[i], b1[i+1], spacing[i/2]) && b1.areLowerIncluded(dir));
      overlap &= b1UpperIncluded && b2.areLowerIncluded(dir);
    }

    i += 2;
  }

  return overlap;
}


//-----------------------------------------------------------------------------
Bounds ESPINA::intersection(const Bounds& b1, const Bounds& b2, NmVector3 spacing)
{
  Bounds res;

  int lo = 0, up = 1, i = 0;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    res[lo] = std::max(b1[lo], b2[lo]);
    res[up] = std::min(b1[up], b2[up]);

    bool li = false;
    if (areEqual(b1[lo], b2[lo], spacing[i]))
      li = b1.areLowerIncluded(dir) && b2.areLowerIncluded(dir);
    else if (areEqual(b1[up], b2[lo], spacing[i]))
      li = b1.areUpperIncluded(dir) && b2.areLowerIncluded(dir);
    else if (areEqual(b1[lo], b2[up], spacing[i]))
      li = b1.areLowerIncluded(dir) && b2.areUpperIncluded(dir);
    else if (b1[lo] < b2[lo])
      li = b2.areLowerIncluded(dir);
    else
      li = b1.areLowerIncluded(dir);
    res.setLowerInclusion(dir, li);

    bool ui = false;
    if (areEqual(b1[up], b2[up], spacing[i]))
      ui = b1.areUpperIncluded(dir) && b2.areUpperIncluded(dir);
    else if (areEqual(b1[up], b2[lo], spacing[i]))
      ui = b1.areUpperIncluded(dir) && b2.areLowerIncluded(dir);
    else if (areEqual(b1[lo], b2[up], spacing[i]))
      ui = b1.areLowerIncluded(dir) && b2.areUpperIncluded(dir);
    else if (b1[up] < b2[up])
      ui = b1.areUpperIncluded(dir);
    else
      ui = b2.areUpperIncluded(dir);
    res.setUpperInclusion(dir, ui);

    ++i;
    lo += 2;
    up += 2;
  }

  return res;

}

//-----------------------------------------------------------------------------
Bounds ESPINA::boundingBox(const Bounds& b1, const Bounds& b2, NmVector3 spacing)
{
  Bounds bb;

  //for(int min = 0, max = 1; min < 6; min += 2, max +=2)
  int min = 0, max = 1, i = 0;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    bb[min] = std::min(b1[min], b2[min]);
    bb[max] = std::max(b1[max], b2[max]);

    bb.setLowerInclusion(dir, areEqual(b1[min], bb[min], spacing[i])?b1.areLowerIncluded(dir):b2.areLowerIncluded(dir));
    bb.setUpperInclusion(dir, areEqual(b1[max], bb[max], spacing[i])?b1.areUpperIncluded(dir):b2.areUpperIncluded(dir));

    ++i;
    min += 2;
    max += 2;
  }

  return bb;
}

//-----------------------------------------------------------------------------
void ESPINA::updateBoundingBox(Bounds &boundingBox, const Bounds &bounds)
{
  if (boundingBox.areValid())
  {
    for (int i = 0, j = 1; i < 6; i += 2, j += 2)
    {
      boundingBox[i] = std::min(bounds[i], boundingBox[i]);
      boundingBox[j] = std::max(bounds[j], boundingBox[j]);
    }
  }
  else
  {
    boundingBox = bounds;
  }
}

//-----------------------------------------------------------------------------
std::ostream& ESPINA::operator<<(std::ostream& os, const Bounds& bounds)
{
  os << bounds.toString().toStdString();

  return os;
}

//-----------------------------------------------------------------------------
QDebug ESPINA::operator<< (QDebug d, const Bounds &bounds)
{
  char borders[6];
  for(auto i: { 0,1,2 })
  {
    if (bounds.areLowerIncluded(toAxis(i)))
      borders[2*i] = '[';
    else
      borders[2*i] = '(';

    if (bounds.areUpperIncluded(toAxis(i)))
      borders[(2*i)+1] = ']';
    else
      borders[(2*i)+1] = ')';
  }

  d << borders[0] << bounds[0] << "," << bounds[1] << borders[1] << borders[2] << bounds[2] << "," << bounds[3] << borders[3] << borders[4] << bounds[4] << "," << bounds[5] << borders[5];
  return d;
}


//-----------------------------------------------------------------------------
bool ESPINA::contains(const Bounds& container, const Bounds& contained, const NmVector3& spacing)
{
  //return intersection(container, contained) == contained;

  int i = 0;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    if (areEqual(contained[i], container[i], spacing[i/2]))
    {
      if (!container.areLowerIncluded(dir) && contained.areLowerIncluded(dir))
      {
	return false;
      }
    } else if (contained[i] < container[i]) {
      return false;
    }
    ++i;
    if (areEqual(contained[i], container[i], spacing[i/2]))
    {
      if (!container.areUpperIncluded(dir) && contained.areUpperIncluded(dir))
      {
	return false;
      }
    } else if (container[i] < contained[i]) {
      return false;
    }
    ++i;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool ESPINA::contains(const Bounds& bounds, const NmVector3& point, const NmVector3& spacing)
{
  int i = 0;
  int j = 0;

  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    if (point[j] < bounds[i]) {
      return false;
    } else if (areEqual(point[j], bounds[i], spacing[j]) && !bounds.areLowerIncluded(dir)) {
      return false;
    }
    ++i;
    if (bounds[i] < point[j]) {
      return false;
    } else if (areEqual(point[j], bounds[i], spacing[j]) && !bounds.areUpperIncluded(dir)) {
      return false;
    }
    ++i;
    ++j;
  }

  return true;

}


//-----------------------------------------------------------------------------
bool ESPINA::operator==(const Bounds &lhs, const Bounds &rhs)
{
  for (int i = 0; i < 6; ++i) {
    if (!areEqual(lhs[i], rhs[i])) return false;
  }

  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    if (lhs.areLowerIncluded(dir) != rhs.areLowerIncluded(dir)
     || lhs.areUpperIncluded(dir) != rhs.areUpperIncluded(dir))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool ESPINA::operator!=(const Bounds &lhs, const Bounds &rhs)
{
  return !(lhs == rhs);
}

//-----------------------------------------------------------------------------
bool ESPINA::areAdjacent(const Bounds &lhs, const Bounds &rhs)
{
  int coincident = 0;
  int adjacent = 0;

  for(int i = 0; i < 3; ++i)
  {
    if (areEqual(lhs[2*i],rhs[2*i+1]) || areEqual(lhs[2*i+1],rhs[2*i]))
      ++adjacent;
    else
      if (areEqual(lhs[2*i], rhs[2*i]) && areEqual(lhs[2*i+1], rhs[2*i+1]))
        ++coincident;
  }

  return ((coincident == 2) && (adjacent == 1));
}
