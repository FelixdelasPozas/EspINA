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

#ifndef ESPINA_CACHED_REPRESENTATION_TASK_H_
#define ESPINA_CACHED_REPRESENTATION_TASK_H_

// EspINA
#include "CachedRepresentation.h"
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/MultiTasking/Task.h>
#include <Core/MultiTasking/Scheduler.h>

// Qt
#include <QColor>

// VTK
#include <vtkSmartPointer.h>

class vtkImageActor;

namespace EspINA
{
  
  class CachedRepresentationTask
  : public Task
  {
    Q_OBJECT
    public:
      /* \brief Default Class constructor.
       *
       */
      explicit CachedRepresentationTask(SchedulerSPtr scheduler)
      : Task{scheduler}
      , m_executionTime{0}
      {};

      /* \brief Class virtual destructor.
       *
       */
      virtual ~CachedRepresentationTask() {};

      /* \brief Sets task input values.
       *
       *  The values must be valid to guarantee that an actor will be generated.
       */
      virtual void setInput(DefaultVolumetricDataSPtr data, Nm position, Plane plane, double brightness, double contrast, QColor color, NmVector3 depth, CachedRepresentation::CacheNode *node) = 0;

    signals:
      void render(CachedRepresentation::CacheNode *);

    protected:
      /* \brief Method to override with the task code.
       *
       *  Method to override with the task code. Input must be set first.
       */
      virtual void run() = 0;

    protected:
      unsigned long long             m_executionTime;
  };

  using CachedRepresentationTaskPtr  = CachedRepresentationTask *;
  using CachedRepresentationTaskSPtr = std::shared_ptr<CachedRepresentationTask>;

} // namespace EspINA

#endif // ESPINA_CACHED_REPRESENTATION_TASK_H_
