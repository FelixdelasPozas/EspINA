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

#include <GUI/Representations/RepresentationManager.h>

#include "CountingFrameManager.h"

namespace ESPINA
{
  namespace CF
  {
    class RepresentationManager3D
    : public GUI::Representations::RepresentationManager
    {
      Q_OBJECT
    public:
      explicit RepresentationManager3D(CountingFrameManager &manager, ViewTypeFlags supportedViews);

      virtual ~RepresentationManager3D();

      virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

    protected:
      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override
      { return false; }

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override
      { return false; }

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override
      { return false; }

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
      virtual bool hasRepresentations() const override;

    virtual void updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void onShow(TimeStamp t) override;

      virtual void onHide(TimeStamp t) override;

      virtual void displayRepresentations(TimeStamp t) override;

      virtual void hideRepresentations(TimeStamp t) override;

      virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

      vtkCountingFrame3DWidget *createWidget(CountingFrame *cf);

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
