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

#ifndef ESPINA_CF_REPRESENTATION_MANAGER_2D_H
#define ESPINA_CF_REPRESENTATION_MANAGER_2D_H

// ESPINA
#include <GUI/Representations/RangedActorManager.h>
#include <GUI/Representations/RepresentationsRange.hxx>
#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class RepresentationManager2D
    : public RepresentationManager
    , public ESPINA::RepresentationManager2D
    {
      Q_OBJECT
    public:
      explicit RepresentationManager2D(CountingFrameManager &manager, ViewTypeFlags supportedViews);

      virtual ~RepresentationManager2D();

      virtual void setResolution(const NmVector3 &resolution);

      virtual PipelineStatus pipelineStatus() const;

      virtual TimeRange readyRange() const;

      virtual void display(TimeStamp t);

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const;

      virtual void setPlane(Plane plane);

      virtual void setRepresentationDepth(Nm depth) {}

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
      virtual void onShow();

      virtual void onHide();

      virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp t);

      virtual RepresentationManagerSPtr cloneImplementation();

      Nm slicingPosition(TimeStamp t) const;

    private:
      Plane     m_plane;
      NmVector3 m_resolution;
      RepresentationsRange<NmVector3> m_crosshairs;

      CountingFrameManager &m_manager;
      QMap<CountingFrame *, vtkCountingFrameSliceWidget *> m_insertedCFs;
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_2D_H
