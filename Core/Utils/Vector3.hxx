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

#ifndef ESPINA_VECTOR_3_H
#define ESPINA_VECTOR_3_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/EspinaException.h>

// C++
#include <iostream>

// Qt
#include <QDebug>
#include <QList>
#include <QString>
#include <QStringList>

namespace ESPINA
{
  /** \brief Set of values defining a point in the 3D space
   *
   */
  template<typename T>
  class EspinaCore_EXPORT Vector3
  {
    public:
      /** \brief Vector3 class constructor.
       *
       */
      Vector3();

      /** \brief Vector3 class constructor.
       * \param[in] value initial list of values.
       *
       */
      Vector3(std::initializer_list<T> values);

      /** \brief Vector3 class constructor.
       * \param[in] string string in the format {%1,%2,%3}.
       *
       */
      explicit Vector3(const QString& string);

      /** \brief Vector3 class constructor.
       * \param[in] point coordinates given by a dim 3 array.
       *
       */
      explicit Vector3(T *point);

      /** \brief Vector3 operator[int]
       *
       */
      T& operator[](int idx)
      { return m_values[idx]; }

      /** \brief Vector3 operator[int] const
       *
       */
      const T& operator[](int idx) const
      { return m_values[idx]; }

      /** \brief Vector3 operator[axis]
       *
       */
      T& operator[](const Axis dir)
      { return m_values[idx(dir)]; }

      /** \brief Vector3 operator[axis] const
       *
       */
      const T& operator[](const Axis dir) const
      { return m_values[idx(dir)]; }

      /** \brief Returns the modulus of the vector (vector magnitude).
       *
       */
      double modulus() const;

      /** \brief Dumps the contents of the vector formatted to a string.
       *
       */
      QString toString() const;
    private:
      T m_values[3];
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  Vector3<T>::Vector3()
  : m_values{0, 0, 0}
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Vector3<T>::Vector3(const QString& string)
  : m_values{0, 0, 0}
  {
    if (string.left(1)  != "{")
    {
      auto what = QObject::tr("Invalid initial string token, token: %1").arg(string.left(1));
      auto details = QObject::tr("Vector3::constructor(QString) -> Invalid initial string token, token: %1").arg(string.left(1));

      throw Core::Utils::EspinaException(what, details);
    }

    if (string.right(1) != "}")
    {
      auto what = QObject::tr("Invalid final string token, token: %1").arg(string.right(1));
      auto details = QObject::tr("Vector3::constructor(QString) -> Invalid final string token, token: %1").arg(string.right(1));

      throw Core::Utils::EspinaException(what, details);
    }

    auto values = string.mid(1,string.length()-2).split(",");

    if (values.size() != 3)
    {
      auto what = QObject::tr("Invalid string items size, size: %1").arg(values.size());
      auto details = QObject::tr("Vector3::constructor(QString) -> Invalid string items size, size: %1").arg(values.size());

      throw Core::Utils::EspinaException(what, details);
    }

    for (int i = 0; i < 3; ++i)
    {
      m_values[i] = values[i].toDouble();
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Vector3<T>::Vector3(std::initializer_list<T> values)
  {
    int i = 0;
    if (values.size() != 3)
    {
      auto what = QObject::tr("Invalid initializer list size, size: %1").arg(values.size());
      auto details = QObject::tr("Vector3::constructor(initializer list) -> Invalid initializer list size, size: %1").arg(values.size());

      throw Core::Utils::EspinaException(what, details);
    }

    for (auto v=values.begin(); v!=values.end(); ++v, ++i)
    {
      m_values[i] = *v;
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Vector3<T>::Vector3(T *point)
  : Vector3{point[0], point[1], point[2]}
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  QString Vector3<T>::toString() const
  {
    return QString("{%1,%2,%3}").arg(m_values[0])
                                .arg(m_values[1])
                                .arg(m_values[2]);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  double Vector3<T>::modulus() const
  {
    return std::sqrt(std::pow(m_values[0],2) + std::pow(m_values[0],2) + std::pow(m_values[0],2));
  }

  /** \brief Multiplies each component of lhs by the corresponding one of rhs
   *
   */
  template<typename T>
  Vector3<T> operator*(const Vector3<T> &lhs, const Vector3<T> &rhs)
  {
    return {lhs[0]*rhs[0], lhs[1]*rhs[1], lhs[2]*rhs[2]};
  }

  /** \brief Divides each component of lhs by the corresponding one of rhs
   *
   */
  template<typename T>
  Vector3<T> operator/(const Vector3<T> &lhs, const Vector3<T> &rhs)
  {
    return {lhs[0]/rhs[0], lhs[1]/rhs[1], lhs[2]/rhs[2]};
  }

  /** \brief Vector3 operator<< for streams.
   *
   */
  template<typename T>
  std::ostream& operator<<(std::ostream& os, const Vector3<T>& vector)
  {
    os << vector.toString().toStdString();

    return os;
  }

  /** \brief Vector3 equality operator.
   *
   */
  template<typename T>
  bool operator==(const Vector3<T> &lhs, const Vector3<T> &rhs)
  {
    for (int i = 0; i < 3; ++i) {
      if (!areEqual(lhs[i], rhs[i])) return false;
    }

    return true;
  }

  /** \brief Vector3 inequality operator.
   *
   */
  template<typename T>
  bool operator!=(const Vector3<T> &lhs, const Vector3<T> &rhs)
  {
    return !(lhs == rhs);
  }

  /** \brief Vector3 operator<< for QDebug.
   *
   */
  template<typename T>
  QDebug operator<<(QDebug d, const Vector3<T> &vector)
  {
    d << "{" << vector[0] << "," << vector[1] << "," << vector[2] << "}";
    return d;
  }

  /** \brief Vector3 operator less
   *
   */
  template<typename T>
  bool operator<(const Vector3<T> &lhs, const Vector3<T> &rhs)
  {
    return ((lhs[0] < rhs[0]) ||
            (lhs[0] == rhs[0] && lhs[1] < rhs[1]) ||
            (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] < rhs[2]));
  }

  using Nm         = double;
  using NmVector3  = Vector3<Nm>;
  using lliVector3 = Vector3<long long int>;
}

#endif // ESPINA_VECTOR_3_H
