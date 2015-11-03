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

#include <Core/Utils/Vector3.hxx>

namespace ESPINA
{
  /** \class RepresentationState
   *
   * This class is not ThreadSafe
   */
  class RepresentationState
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


  bool hasCrosshairPoint(const RepresentationState &state);

  /** \brief Sets the crosshair point position for this representation
   * \param[in] point crosshair point
   *
   */
  void setCrosshairPoint(const NmVector3 &point, RepresentationState &state);

  /** \brief Returns the crosshair point for this representation.
   *
   */
  NmVector3 crosshairPoint(const RepresentationState &state);

  Nm crosshairPosition(const Plane &plane, const RepresentationState &state);

  /** \brief Returns if the crosshair point has been modified
   *
   */
  bool isCrosshairPointModified(const RepresentationState &state);

  bool isCrosshairPositionModified(const Plane &plane, const RepresentationState &state);

  bool isVisible(const RepresentationState &state);

  QDebug operator<<(QDebug debug, const RepresentationState &state);
}

#endif // ESPINA_REPRESENTATIONSTATE_H
