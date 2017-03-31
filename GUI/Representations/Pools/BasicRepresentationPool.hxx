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

// ESPINA
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Frame.h>

// VTK
#include <vtkProp.h>

namespace ESPINA
{
  /** \class BasicRepresentationPool
   * \brief Basic representation pool without a cache. Updates all the items in a single thread and
   * it's intended to use with those views that don't update it's actors when the crosshair changes,
   * like the 3D views.
   *
   * This is templated over the representation updater:
   *  - RepresentationUpdater: for single thread actors creation/modification.
   *  - RepresentationParallelUpdater: for parallel actor creation/modification.
   *
   *  Use parallel for those managers that needs a lot of actors being created each time a representation
   *  parameter changes, like meshes. Use the single thread for fast and small amount of actors.
   *
   */
  template<class T>
  class BasicRepresentationPool
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
      std::shared_ptr<T>         m_updater;     /** pool's representation updater.             */
      QString                    m_description; /** updater's description                      */
      RepresentationPipelineSPtr m_pipeline;    /** actor creation pipeline and pick resolver. */
  };

  //-----------------------------------------------------------------------------
  template<class T>
  BasicRepresentationPool<T>::BasicRepresentationPool(const ItemAdapter::Type &type, SchedulerSPtr scheduler, RepresentationPipelineSPtr pipeline)
  : RepresentationPool{type}
  , m_updater         {std::make_shared<T>(scheduler, pipeline)}
  , m_description     {pipeline->type()}
  , m_pipeline        {pipeline}
  {
    connect(m_updater.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)),
            this,            SLOT(onActorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)), Qt::DirectConnection);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  ViewItemAdapterList BasicRepresentationPool<T>::pick(const NmVector3 &point, vtkProp *actor) const
  {
    ViewItemAdapterList result;

    auto lastActors = actors(lastUpdateTimeStamp());

    if(lastActors.get() != nullptr)
    {
      ViewItemAdapterPtr pickedItem = nullptr;

      {
        RepresentationPipeline::ActorsLocker frameActors(lastActors, true);

        if(frameActors.isLocked())
        {
          if (actor)
          {
            auto it = frameActors.get().begin();

            while (it != frameActors.get().end() && !pickedItem)
            {
              for (auto itemActor : it.value())
              {
                if (itemActor.GetPointer() == actor)
                {
                  pickedItem = it.key();
                  break;
                }
              }

              ++it;
            }
          }
          else
          {
            for (auto item: frameActors.get().keys())
            {
              if (m_pipeline->pick(item, point))
              {
                pickedItem = item;
                break;
              }
            }
          }
        }
      }

      if (pickedItem && m_pipeline->pick(pickedItem, point))
      {
        result << pickedItem;
      }
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::updatePipelinesImplementation(const GUI::Representations::FrameCSPtr frame)
  {
    m_updater->invalidate();
    m_updater->setCrosshair(frame->crosshair);
    m_updater->setResolution(frame->resolution);
    m_updater->setFrame(frame);
    m_updater->setDescription(m_description);

    updateRepresentations();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::updateRepresentationsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
  {
    m_updater->setFrame(frame);
    m_updater->updateRepresentations(modifiedItems);

    updateRepresentations();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::updateRepresentationColorsAtImlementation(const GUI::Representations::FrameCSPtr frame, ViewItemAdapterList modifiedItems)
  {
    m_updater->setFrame(frame);
    m_updater->updateRepresentationColors(modifiedItems);

    updateRepresentations();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::addRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->addSource(source);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::removeRepresentationPipeline(ViewItemAdapterPtr source)
  {
    m_updater->removeSource(source);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::applySettings(const RepresentationState &settings)
  {
    m_updater->setSettings(settings);

    updateRepresentations();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void BasicRepresentationPool<T>::updateRepresentations()
  {
    if(isEnabled())
    {
      emit taskStarted(m_updater);
      Task::submit(m_updater);
    }
  }
}

#endif // ESPINA_BASIC_REPRESENTATION_POOL_H
