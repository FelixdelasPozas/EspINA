/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SLICE_MANAGER_H
#define ESPINA_SLICE_MANAGER_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Managers/PoolManager.h>

namespace ESPINA
{
  /** \class SliceManager
   * \brief Manager for segmentation slice representations.
   *
   */
  class EspinaSupport_EXPORT SliceManager
  : public GUI::Representations::Managers::PoolManager
  , public GUI::Representations::RepresentationManager2D
  {
    public:
      /** \brief SliceManager class constructor.
       * \param[in] poolXY pool for the Axial plane.
       * \param[in] poolXZ pool for the Coronal plane.
       * \param[in] ppolYZ pool for the Sagittal plane.
       * \param[in] flags manager's flags values.
       *
       */
      explicit SliceManager(RepresentationPoolSPtr poolXY,
                            RepresentationPoolSPtr poolXZ,
                            RepresentationPoolSPtr poolYZ,
                            ManagerFlags           flags = ManagerFlags());

      /** \brief SliceManager class virtual destructor.
       *
       */
      virtual ~SliceManager()
      {};

      virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

      virtual void setPlane(Plane plane) override;

      virtual void setRepresentationDepth(Nm depth) override;

      virtual RepresentationPoolSList pools() const override;

    protected:
      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

    private:
      virtual bool hasRepresentations() const override;

      virtual void updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame) override;

      virtual RepresentationPipeline::Actors actors(TimeStamp time) override;

      virtual void invalidatePreviousActors(TimeStamp time) override;

      virtual void onShow(const GUI::Representations::FrameCSPtr frame) override;

      virtual void onHide(const GUI::Representations::FrameCSPtr frame) override;

      virtual GUI::Representations::RepresentationManagerSPtr cloneImplementation() override;

      /** \brief Helper method to connect the pool's signals.
       *
       */
      void connectPools();

      /** \brief Helper method to disconnect the pool's signals.
       *
       */
      void disconnectPools();

      /** \brief Returns the pool corresponding to the manager's plane.
       *
       */
      RepresentationPoolSPtr planePool() const;

      /** \brief Returns true if the manager has a valid plane configured.
       *
       */
      bool validPlane() const;

      /** \brief Returns the normal coordinate for the given point according to the configured plane.
       * \param[in] point point coordinates.
       *
       */
      Nm normalCoordinate(const NmVector3 &point) const;

    private:
      Plane                  m_plane; /** plane of the manager.                                         */
      Nm                     m_depth; /** distance from the plane position to show the representations. */
      RepresentationPoolSPtr m_XY;    /** Axial plane pool.                                             */
      RepresentationPoolSPtr m_XZ;    /** Coronal plane pool.                                           */
      RepresentationPoolSPtr m_YZ;    /** Sagittal plane pool.                                          */
  };
}

#endif // ESPINA_SLICE_MANAGER_H
