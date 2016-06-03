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

#ifndef ESPINA_SLICE_3D_MANAGER_H
#define ESPINA_SLICE_3D_MANAGER_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <GUI/Representations/Managers/PoolManager.h>
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  class EspinaSupport_EXPORT Slice3DManager
  : public GUI::Representations::Managers::PoolManager
  {
      Q_OBJECT
    public:
      /** \brief Slice3DManager class constructor.
       * \param[in] poolXY pool for the Axial plane.
       * \param[in] poolXZ pool for the Coronal plane.
       * \param[in] ppolYZ pool for the Sagittal plane.
       * \param[in] flags manager's flags values.
       *
       */
      explicit Slice3DManager(RepresentationPoolSPtr poolXY,
                              RepresentationPoolSPtr poolXZ,
                              RepresentationPoolSPtr poolYZ,
                              ManagerFlags           flags = ManagerFlags());

      /** \brief Slice3DManager class virtual destructor.
       *
       */
      virtual ~Slice3DManager()
      {};

      virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

      virtual RepresentationPoolSList pools() const override;

    protected:
      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const override;

    private:
      virtual bool hasRepresentations() const override;

      virtual void updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual void onShow(const GUI::Representations::FrameCSPtr frame) override;

      virtual void onHide(const GUI::Representations::FrameCSPtr frame) override;

      virtual RepresentationPipeline::Actors actors(TimeStamp time) override;

      virtual void invalidatePreviousActors(TimeStamp time) override;

      virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

      /** \brief Helper method to connect the pool's signals.
       *
       */
      void connectPools();

      /** \brief Helper method to disconnect the pool's signals.
       *
       */
      void disconnectPools();

    private slots:
      /** \brief Helper method to emit a render request only when all the pools have the actors ready for the given frame.
       * \param[in] frame const frame object.
       *
       */
      void checkRenderRequest(const GUI::Representations::FrameCSPtr frame);

    private:
      RepresentationPoolSList m_pools; /** list of managed pools. */
  };
}

#endif // ESPINA_SLICE_3D_MANAGER_H
