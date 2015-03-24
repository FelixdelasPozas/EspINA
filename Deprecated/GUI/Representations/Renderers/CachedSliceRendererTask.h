/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_CACHED_SLICE_RENDERER_TASK_H_
#define ESPINA_CACHED_SLICE_RENDERER_TASK_H_

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Deprecated/GUI/Representations/Renderers/CachedSliceRenderer.h>
#include <Deprecated/GUI/Representations/SliceCachedRepresentation.h>
#include <QMap>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT CachedSliceRendererTask
  : public Task
  {
    Q_OBJECT
    public:
      /** \brief CachedRepresentationTask class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       *
       */
      explicit CachedSliceRendererTask(SchedulerSPtr scheduler);

      /** \brief CachedRepresentationTask class virtual destructor.
       *
       */
      virtual ~CachedSliceRendererTask();

      /** \brief Sets task input values.
       * \param[in] node, pointer to the node where the actors will be placed.
       * \param[in] repList, list of representations.
       *
       *  The values must be valid to guarantee that an actor will be generated.
       */
      virtual void setInput(CachedSliceRenderer::CacheNode *node, CachedRepresentationSList representations);

    signals:
      void ready(CachedSliceRenderer::CacheNode *);

    protected:
      /** \brief Implements Task::run().
       *
       */
      virtual void run();

    private:
      /** \brief Compute the actor for that representation in the specified position.
       * \param[in] rep, cached representation smart pointer.
       *
       */
      void computeData(CachedRepresentationSPtr rep);

      /** \brief Deallocates all actors.
       *
       */
      void releaseActors();

      /** \brief Checks the node flag and returns true if the task must restart its work.
       *
       */
      bool needToRestart();

      QMap<CachedRepresentationSPtr, vtkSmartPointer<vtkImageActor>> m_representations;
      CachedSliceRenderer::CacheNode *m_node;
      Nm                              m_position;
  };

  using CachedSliceRendererTaskPtr  = CachedSliceRendererTask *;
  using CachedSliceRendererTaskSPtr = std::shared_ptr<CachedSliceRendererTask>;

} // namespace ESPINA

#endif // ESPINA_CACHED_SLICE_RENDERER_TASK_H_
