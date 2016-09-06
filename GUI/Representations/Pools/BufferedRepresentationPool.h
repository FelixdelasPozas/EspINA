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

#ifndef ESPINA_BUFFERED_REPRESENTATION_POOL_H
#define ESPINA_BUFFERED_REPRESENTATION_POOL_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/RepresentationUpdater.h>
#include <GUI/Representations/RepresentationWindow.h>

// VTK
#include <vtkMath.h>

namespace ESPINA
{
  /** \class BufferedRepresentationPool
   * \brief Buffered representation pool for 2D representations.
   *
   */
  class EspinaGUI_EXPORT BufferedRepresentationPool
  : public RepresentationPool
  {
    public:
      /** \brief BufferedRepresentationPool class constructor.
       * \param[in] type type of the managed representations.
       * \param[in] plane normal plane of the pool.
       * \param[in] pipeline generator of the actors for the items.
       * \param[in] scheduler task scheduler.
       * \param[in] windowsize width/2 of the window of the pool.
       *
       */
      explicit BufferedRepresentationPool(const ItemAdapter::Type   &type,
                                          const Plane                plane,
                                          RepresentationPipelineSPtr pipeline,
                                          SchedulerSPtr              scheduler,
                                          unsigned                   windowSize);

      virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

    private:
      virtual void updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame);

      virtual void updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

      virtual void updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

      virtual void addRepresentationPipeline(ViewItemAdapterPtr source) override;

      virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) override;

      virtual void applySettings(const RepresentationState &settings) override;

      /** \brief Updates the priorities of the task depending on the position.
       *
       */
      void updatePriorities();

      /** \brief Returns the distance in resolution steps from the crosshair.
       * \param[in] crosshair crosshair point.
       *
       */
      int distanceFromLastCrosshair(const NmVector3 &crosshair);

      /** \brief Returns the value of the point corresponding with the normal plane.
       * \param[in] point point coordinates.
       *
       */
      Nm normalCoordinate(const NmVector3 &point) const;

      /** \brief Returns the crosshair modified acording to the shift in resolution steps.
       * \param[in] point crosshair point.
       * \param[in] shift resolution steps from the crosshair point.
       *
       */
      NmVector3 representationCrosshair(const NmVector3 &point, int shift) const;

      /** \brief Configures and return a list of invalid updaters ready to be executed
       *
       */
      RepresentationUpdaterSList updateBuffer(const NmVector3 &point, int shift, const GUI::Representations::FrameCSPtr frame);

      void updatePipelines(RepresentationUpdaterSList updaters);

      /** \brief Checks if the current actors are ready.
       *
       */
      void checkCurrentActors();

      /** \brief Returns the invalidation shift for the current window size.
       *
       */
      int invalidationShift() const;

    private:
      const int            m_normalIdx;    /** index of the plane normal. in [0,1,2] */
      RepresentationWindow m_updateWindow; /** the tasks window.                     */
      Nm                   m_normalRes;    /** resolution of the normal plane.       */
      NmVector3            m_crosshair;    /** current position's crosshair.         */
  };
}

#endif // ESPINA_BUFFERED_REPRESENTATION_POOL_H
