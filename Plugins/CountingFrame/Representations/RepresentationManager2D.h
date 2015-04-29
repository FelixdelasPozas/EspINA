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
#include <GUI/Representations/RepresentationManager.h>
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

      virtual TimeRange readyRangeImplementation() const override;

      virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const override;

      virtual void setPlane(Plane plane) override;

      virtual void setRepresentationDepth(Nm depth) override
      {}

    protected:
      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const        { return false; };

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const { return false; };

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const            { return false; };

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
      virtual bool hasRepresentations() const { return false; };

      virtual void updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t) {};

      virtual void onShow(TimeStamp t);

      virtual void onHide(TimeStamp t);

      virtual void displayRepresentations(TimeStamp t) {};

      virtual void hideRepresentations(TimeStamp t) {};

      virtual RepresentationManagerSPtr cloneImplementation();

      Nm slicingPosition(TimeStamp t) const;

      vtkCountingFrameSliceWidget *createWidget(CountingFrame *cf);

      void showWidget(vtkCountingFrameSliceWidget *widget);

      void hideWidget(vtkCountingFrameSliceWidget *widget);

      void deleteWidget(CountingFrame *cf);

      bool isNormalDifferent(const NmVector3 &p1, const NmVector3 &p2) const;

    private:
      Plane     m_plane;
      NmVector3 m_resolution;
      RangedValue<NmVector3> m_crosshairs;

      CountingFrameManager  &m_manager;
      QList<CountingFrame *> m_pendingCFs;
      QMap<CountingFrame *, vtkCountingFrameSliceWidget *> m_widgets;
    };
  }
}

#endif // ESPINA_CF_REPRESENTATION_MANAGER_2D_H
