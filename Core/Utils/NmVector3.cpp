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

#include "NmVector3.h"

#include <QStringList>

using namespace EspINA;

//-----------------------------------------------------------------------------
NmVector3::NmVector3()
: m_values{0, 0, 0}
{
}

//-----------------------------------------------------------------------------
NmVector3::NmVector3(const QString& string)
: m_values{0, 0, 0}
{
  if (string.left(1)  != "{") throw Wrong_format_exception();
  if (string.right(1) != "}") throw Wrong_format_exception();

  auto values = string.mid(1,string.length()-2).split(",");

  if (values.size() != 3) throw Wrong_format_exception();

  for (int i = 0; i < 3; ++i)
  {
    m_values[i] = values[i].toDouble();
  }
}

//-----------------------------------------------------------------------------
NmVector3::NmVector3(std::initializer_list<Nm> values)
{
  int i = 0;
  if (values.size() != 3) throw Wrong_number_initial_values();

  for (auto v=values.begin(); v!=values.end(); ++v, ++i) {
        m_values[i] = *v;
  }
}

//-----------------------------------------------------------------------------
QString NmVector3::toString() const
{
  return QString("{%1,%2,%3}").arg(m_values[0])
                              .arg(m_values[1])
                              .arg(m_values[2]);
}

//-----------------------------------------------------------------------------
std::ostream& EspINA::operator<<(std::ostream& os, const NmVector3& vector)
{
  os << vector.toString().toStdString();

  return os;
}



//-----------------------------------------------------------------------------
bool EspINA::operator==(const NmVector3 &lhs, const NmVector3 &rhs)
{
  for (int i = 0; i < 3; ++i) {
    if (!areEqual(lhs[i], rhs[i])) return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool EspINA::operator!=(const NmVector3 &lhs, const NmVector3 &rhs)
{
  return !(lhs == rhs);
}

//-----------------------------------------------------------------------------
QDebug EspINA::operator<< (QDebug d, const NmVector3 &vector)
{
  d << "{" << vector[0] << "," << vector[1] << "," << vector[2] << "}";
  return d;
}
