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

#ifndef ESPINA_BASIC_REPRESENTATION_POOL_H
#define ESPINA_BASIC_REPRESENTATION_POOL_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/RepresentationUpdater.h>

namespace ESPINA
{
  /** \class BasicRepresentationPool
   * \brief Basic representation pool without a cache. Updates all the items in a single thread and
   * it's intended to use with those views that don't update it's actors when the crosshair changes,
   * like the 3D views.
   *
   */
  class EspinaGUI_EXPORT BasicRepresentationPool
  : public RepresentationPool
  {
    public:
      /** \brief BasicRepresentationPool class constructor.
       * \param[in] type type of the items being managed.
       * \param[in] scheduler task scheduler for launching the updater.
       * \param[in] pipeline representation updater.
       *
       */
      explicit BasicRepresentationPool(const ItemAdapter::Type &type, SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline);

      virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

    private:
      virtual void updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame) override;

      virtual void updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

      virtual void updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems) override;

      virtual void addRepresentationPipeline(ViewItemAdapterPtr source) override;

      virtual void removeRepresentationPipeline(ViewItemAdapterPtr source) override;

      virtual void applySettings(const RepresentationState &settings) override;

    private:
      /** brief Helper method to update the items' representations.
       *
       */
      void updateRepresentations();

    private:
      RepresentationUpdaterSPtr m_updater; /** pool's representation updater. */
      QString m_description;               /** updater's description          */
  };
}

#endif // ESPINA_BASIC_REPRESENTATION_POOL_H
