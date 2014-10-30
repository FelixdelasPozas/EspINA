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

#ifndef ESPINA_NM_VECTOR_3_H
#define ESPINA_NM_VECTOR_3_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Utils/Spatial.h"

// C++
#include <iostream>

// Qt
#include <QDebug>
#include <QList>
#include <QString>

namespace ESPINA {

  using Nm = double;

  /** \brief Set of values defining a point in the 3D space
   *
   */
  class EspinaCore_EXPORT NmVector3
  {
  public:
    struct Wrong_Format_Exception {};
    struct Wrong_number_initial_values {};

  public:
    /** \brief NmVector3 class constructor.
     *
     */
    NmVector3();

    /** \brief NmVector3 class constructor.
     * \param[in] value initial list of values.
     *
     */
    NmVector3(std::initializer_list<Nm> values);

    /** \brief NmVector3 class constructor.
     * \param[in] string string in the format {%1,%2,%3}.
     *
     */
    explicit NmVector3(const QString& string);

    /** \brief NmVector3 operator[int]
     *
     */
    Nm& operator[](int idx)
    { return m_values[idx]; }

    /** \brief NmVector3 operator[int] const
     *
     */
    const Nm& operator[](int idx) const
    { return m_values[idx]; }

    /** \brief NmVector3 operator[axis]
     *
     */
    Nm& operator[](const Axis dir)
    { return m_values[idx(dir)]; }

    /** \brief NmVector3 operator[axis] const
     *
     */
    const Nm& operator[](const Axis dir) const
    { return m_values[idx(dir)]; }

    /** \brief Dumps the contents of the vector formatted to a string.
     *
     */
    QString toString() const;

  private:
    Nm m_values[3];
  };

  /** \brief NmVector3 operator<< for QDebug.
   *
   */
  QDebug EspinaCore_EXPORT operator<< (QDebug d, const NmVector3 &vector);

  /** \brief NmVector3 operator<< for streams.
   *
   */
  std::ostream& EspinaCore_EXPORT operator<<(std::ostream& os, const NmVector3& vector);

  /** \brief NmVector3 equality operator.
   *
   */
  bool EspinaCore_EXPORT operator==(const NmVector3& lhs, const NmVector3& rhs);

  /** \brief NmVector3 inequality operator.
   *
   */
  bool EspinaCore_EXPORT operator!=(const NmVector3& lhs, const NmVector3& rhs);
}

#endif // ESPINA_NM_VECTOR_3_H
