/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "CachedSliceRenderer.h"
#include <Core/MultiTasking/Task.h>
#include <Core/MultiTasking/Scheduler.h>

// Qt
#include <QMap>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>

namespace EspINA
{
  class CachedSliceRendererTask
  : public Task
  {
    Q_OBJECT
    public:
      /* \brief CachedRepresentationTask Class constructor.
       * \param[in] scheduler Scheduler to sumbit.
       *
       */
      explicit CachedSliceRendererTask(SchedulerSPtr scheduler);

      /* \brief CachedRepresentationTask virtual destructor.
       *
       */
      virtual ~CachedSliceRendererTask();

      /* \brief Sets task input values.
       * \param[in] *node    Pointer to the node where the actors will be placed.
       * \param[in] repList  List of representations.
       *
       *  The values must be valid to guarantee that an actor will be generated.
       */
      virtual void setInput(CachedSliceRenderer::CacheNode *node, RepresentationSList representations);

    signals:
      void ready(CachedSliceRenderer::CacheNode *);

    protected:
      /* \brief Threaded code.
       *
       */
      virtual void run();

    private:
      /* \brief Fill the data struct for that representation.
       *
       */
      void computeData(RepresentationSPtr rep);

      /* \brief Deallocates actors.
       *
       */
      void releaseActors();

      /* \brief Checks the node flag and returns true if the task must restart its work.
       *
       */
      bool needToRestart();

      QMap<RepresentationSPtr, struct CachedSliceRenderer::ActorData> m_representations;

      unsigned long long              m_executionTime;
      CachedSliceRenderer::CacheNode *m_node;
      Nm                              m_position;
  };

  using CachedSliceRendererTaskPtr  = CachedSliceRendererTask *;
  using CachedSliceRendererTaskSPtr = std::shared_ptr<CachedSliceRendererTask>;

} // namespace EspINA

#endif // ESPINA_CACHED_SLICE_RENDERER_TASK_H_
