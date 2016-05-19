/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_REPRESENTATION_STATE_H
#define ESPINA_REPRESENTATION_STATE_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Vector3.hxx>

namespace ESPINA
{
  /** \class RepresentationState
   *
   * This class is not ThreadSafe
   */
  class EspinaGUI_EXPORT RepresentationState
  {
    using Pair = QPair<QVariant, bool>;

  public:
    template<typename T>
    void setValue(const QString &tag, T value)
    {
      Pair pair = m_properties.value(tag, Pair(QVariant(), true));

      pair.second |= pair.first != value;

      if (pair.second)
      {
        pair.first = value;
        m_properties[tag] = pair;
      }
    }

    template<typename T>
    T getValue(const QString &tag) const
    { return m_properties[tag].first.value<T>(); }

    bool isModified(const QString &tag) const
    { return m_properties.value(tag, Pair(QVariant(), false)).second; }

    bool hasValue(const QString &tag) const;

    bool hasPendingChanges() const;

    void apply(const RepresentationState &state);

    void commit();

    void clear();

    QMap<QString, Pair> m_properties;
  private:
  };

  /** \brief Returs true if the given settings has values for the crosshair point.
   * \param[in] state RepresentationState object reference.
   *
   */
  bool EspinaGUI_EXPORT hasCrosshairPoint(const RepresentationState &state);

  /** \brief Sets the crosshair point position for the given state.
   * \param[in] point crosshair point
   * \param[in] state RepresentationState object reference.
   *
   */
  void EspinaGUI_EXPORT setCrosshairPoint(const NmVector3 &point, RepresentationState &state);

  /** \brief Returns the crosshair point for the given state.
   * \param[in] state RepresentationState object reference.
   *
   */
  NmVector3 EspinaGUI_EXPORT crosshairPoint(const RepresentationState &state);

  /** \brief Returns the value of the crosshair position for the given plane in the given state.
   * \param[in] plane Axis plane.
   * \param[in] state RepresentationState object reference.
   *
   */
  Nm EspinaGUI_EXPORT crosshairPosition(const Plane &plane, const RepresentationState &state);

  /** \brief Returns if the crosshair point has been modified in the given state.
   * \param[in] state RepresentationState object reference.
   *
   */
  bool EspinaGUI_EXPORT isCrosshairPointModified(const RepresentationState &state);

  /** \brief Returns true if the crosshair point have been modified for the given plane in the given state.
   * \param[in] plane Axis plane.
   * \param[in] state RepresentationState object reference.
   *
   */
  bool EspinaGUI_EXPORT isCrosshairPositionModified(const Plane &plane, const RepresentationState &state);

  /** \brief Returns true if the representation is visible in the given state.
   * \param[in] state RepresentationState object reference.
   *
   */
  bool EspinaGUI_EXPORT isVisible(const RepresentationState &state);

  QDebug EspinaGUI_EXPORT operator<<(QDebug debug, const RepresentationState &state);
}

#endif // ESPINA_REPRESENTATIONSTATE_H
