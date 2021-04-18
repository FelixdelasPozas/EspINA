/*
 * Copyright (c) 2014, Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_STATE_PAIR_H
#define ESPINA_STATE_PAIR_H

#include <Core/Utils/Vector3.hxx>
#include <QString>

namespace ESPINA
{
  /** \brief Helper method to create a string in the form "key=value;".
   * \param[in] id Key.
   * \param[in] value Value.
   *
   */
  template<class T>
  QString StatePair(const QString &id, const T &value)
  { return QString("%1=%2;").arg(id).arg(value); }

  /** \brief Helper method to create a string in the form "key=a,b,c;" from NmVector3 values.
   * \param[in] id Key.
   * \param[in] value NmVector3 object.
   *
   */
  inline QString StatePair(const QString &id, const NmVector3 &value)
  { return QString("%1=%2,%3,%4;").arg(id).arg(value[0]).arg(value[1]).arg(value[2]); }
}


#endif // ESPINA_STATE_PAIR_H
