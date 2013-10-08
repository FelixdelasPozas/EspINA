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

#include "Bounds.h"

using namespace EspINA;

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

  throw Invalid_bounds_token();
}

//-----------------------------------------------------------------------------
bool upperBoundsInclusion(double value) {
  if (value == ']') return true;
  if (value == ')') return false;

  throw Invalid_bounds_token();
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
      throw Wrong_size_error{};
  }

}

//-----------------------------------------------------------------------------
bool EspINA::intersect(const Bounds& b1, const Bounds& b2)
{
  auto lessThan         = [](double a, double b){return a <  b;};
  auto lessEqualThan    = [](double a, double b){return a <= b;};
  auto greaterThan      = [](double a, double b){return a >  b;};
  auto greaterEqualThan = [](double a, double b){return a >= b;};

  bool overlap = true;
  int  i = 0;
  for (auto dir : {Axis::X, Axis::Y, Axis::Z}) {
    auto lower = (b1.areLowerIncluded(dir) && b2.areUpperIncluded(dir))?lessEqualThan:lessThan;
    auto upper = (b1.areUpperIncluded(dir) && b2.areLowerIncluded(dir))?greaterEqualThan:greaterThan;
 
    overlap &= lower(b1[i], b2[i+1]) && upper(b1[i+1], b2[i]);
    i += 2; 
  }

  return overlap;
}


//-----------------------------------------------------------------------------
Bounds EspINA::intersection(const Bounds& b1, const Bounds& b2)
{
  Bounds res;

  int lo = 0, up = 1;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    res[lo] = std::max(b1[lo], b2[lo]);
    res[up] = std::min(b1[up], b2[up]);

    bool li = false;
    if (b1[lo] == b2[lo])
      li = b1.areLowerIncluded(dir) && b2.areLowerIncluded(dir);
    else if (b1[up] == b2[lo])
      li = b1.areUpperIncluded(dir) && b2.areLowerIncluded(dir);
    else if (b1[lo] == b2[up])
      li = b1.areLowerIncluded(dir) && b2.areUpperIncluded(dir);
    else if (b1[lo] < b2[lo])
      li = b2.areLowerIncluded(dir);
    else
      li = b1.areLowerIncluded(dir);
    res.setLowerInclusion(dir, li);

    bool ui = false;
    if (b1[up] == b2[up])
      ui = b1.areUpperIncluded(dir) && b2.areUpperIncluded(dir);
    else if (b1[up] == b2[lo])
      ui = b1.areUpperIncluded(dir) && b2.areLowerIncluded(dir);
    else if (b1[lo] == b2[up])
      ui = b1.areLowerIncluded(dir) && b2.areUpperIncluded(dir);
    else if (b1[up] < b2[up])
      ui = b1.areUpperIncluded(dir);
    else
      ui = b2.areUpperIncluded(dir);
    res.setUpperInclusion(dir, ui);

    lo += 2;
    up += 2;
  }

  return res;

}

//-----------------------------------------------------------------------------
Bounds EspINA::boundingBox(const Bounds& b1, const Bounds& b2)
{
  Bounds bb;

  //for(int min = 0, max = 1; min < 6; min += 2, max +=2)
  int min = 0, max = 1;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z})
  {
    bb[min] = std::min(b1[min], b2[min]);
    bb[max] = std::max(b1[max], b2[max]);

    bb.setLowerInclusion(dir, (b1[min] == bb[min])?b1.areLowerIncluded(dir):b2.areLowerIncluded(dir));
    bb.setUpperInclusion(dir, (b1[max] == bb[max])?b1.areUpperIncluded(dir):b2.areUpperIncluded(dir));

    min += 2;
    max += 2;
  }


  return bb;
}


//-----------------------------------------------------------------------------
std::ostream& EspINA::operator<<(std::ostream& os, const Bounds& bounds)
{
  os << "{";
  int i = 0;
  for (auto axis : {Axis::X , Axis::Y, Axis::Z}) {
    if (i > 0) {os << ",";}
    os << (bounds.areLowerIncluded(axis)?"[":"(");
    os << bounds[i] << "," << bounds[i+1];
    os << (bounds.areUpperIncluded(axis)?"]":")");
    i += 2;
  }
  os << "}";

  return os;
}


//-----------------------------------------------------------------------------
bool EspINA::areInside(const Bounds &contained, const Bounds &container)
{
  int i = 0;
  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    if (contained[i] < container[i]) {
      return false;
    } else if (contained[i] == container[i] && !container.areLowerIncluded(dir) && contained.areLowerIncluded(dir)) {
      return false;
    }
    ++i;
    if (container[i] < contained[i]) {
      return false;
    } else if (contained[i] == container[i] && !container.areUpperIncluded(dir) && contained.areUpperIncluded(dir)) {
      return false;
    }
    ++i;
  }

  return true;
}


//-----------------------------------------------------------------------------
bool EspINA::operator==(const Bounds &lhs, const Bounds &rhs)
{
  for (int i = 0; i < 6; ++i) {
    if (lhs[i] != rhs[i]) return false;
  }

  for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
    if (lhs.areLowerIncluded(dir) != rhs.areLowerIncluded(dir) 
     || lhs.areUpperIncluded(dir) != rhs.areUpperIncluded(dir))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool EspINA::operator!=(const Bounds &lhs, const Bounds &rhs)
{
  return !(lhs == rhs);
}
