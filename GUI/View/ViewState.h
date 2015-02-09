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

#ifndef ESPINA_VIEW_STATE_H
#define ESPINA_VIEW_STATE_H

#include <QObject>
#include <Core/Utils/NmVector3.h>
#include <Core/EspinaTypes.h>

namespace ESPINA
{
  class ViewState
  : public QObject
  {
    Q_OBJECT

  public:
    /** \brief Ensure point position is visible
     *
     */
    void focusViewOn(const NmVector3 &point);

    TimeStamp timeStamp() const;

    NmVector3 crosshair() const;

  public slots:
    /** \brief Changes the crosshair position to point
     *
     */
    void setCrosshair(const NmVector3 &point);

    /** \brief Changes the crosshair position of the given plane
     *
     */
    void setCrosshairPlane(const Plane plane, const Nm position);

  signals:
    void crosshairChanged(NmVector3 point, TimeStamp time);

    void viewFocusedOn(NmVector3);

  private:
    TimeStamp m_timeStamp;
    NmVector3 m_crosshair;
  };

  using ViewStateSPtr = std::shared_ptr<ViewState>;
}

#endif // ESPINA_VIEWSTATE_H
