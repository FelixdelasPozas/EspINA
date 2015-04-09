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

#ifndef ESPINA_CF_REPRESENTATION_MANAGER_3D_H
#define ESPINA_CF_REPRESENTATION_MANAGER_3D_H

#include <GUI/Representations/ActorManager.h>

#include <GUI/Representations/RepresentationsRange.hxx>
#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class RepresentationManager3D
    : public RepresentationManager
    {
      Q_OBJECT
    public:
      explicit RepresentationManager3D(CountingFrameManager &manager, ViewTypeFlags supportedViews);

      virtual ~RepresentationManager3D();

      virtual TimeRange readyRange() const override;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const override;

    private slots:
      /** \brief Helper method to update internal data when a CF is created.
       *
       */
      void onCountingFrameCreated(CountingFrame *cf);

      /** \brief Helper method to update internal data when a CF is removed.
       *
       */
      void onCountingFrameDeleted(CountingFrame *cf);

    private:
      virtual void displayImplementation(TimeStamp t) override;

      virtual void onShow(TimeStamp t) override;

      virtual void onHide(TimeStamp t) override;

      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp t) {}

      virtual RepresentationManagerSPtr cloneImplementation();

      Nm slicingPosition(TimeStamp t) const;

      vtkCountingFrameWidget *createWidget(CountingFrame *cf);

      void showWidget(vtkCountingFrameWidget *widget);

      void hideWidget(vtkCountingFrameWidget *widget);

      void deleteWidget(CountingFrame *cf);

    private:
      CountingFrameManager  &m_manager;
      QList<CountingFrame *> m_pendingCFs;
      QMap<CountingFrame *, vtkCountingFrame3DWidget *> m_widgets;
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_3D_H
