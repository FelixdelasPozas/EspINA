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

#ifndef ESPINA_SLICE_CACHED_REPRESENTATION_TASK_H_
#define ESPINA_SLICE_CACHED_REPRESENTATION_TASK_H_

// EspINA
#include "SliceCachedRepresentation.h"
#include "CachedRepresentationTask.h"

// VTK
#include <vtkSmartPointer.h>

class vtkImageMapToColors;
class vtkImageShiftScale;
class vtkImageActor;
class vtkLookupTable;

namespace EspINA
{
  //-----------------------------------------------------------------------------
  class ChannelSliceCachedRepresentationTask
  : public CachedRepresentationTask
  {
    public:
      /* \brief Default class constructor
       *
       */
      explicit ChannelSliceCachedRepresentationTask(SchedulerSPtr scheduler);

      /* \brief Default class destructor
       *
       */
      virtual ~ChannelSliceCachedRepresentationTask()
      {};

      /* \brief Sets task input values.
       *
       *  The values must be valid to guarantee that an actor will be generated.
       */
      void setInput(DefaultVolumetricDataSPtr data, Nm position, Plane plane, double brightness, double contrast, QColor color, NmVector3 depth, CachedRepresentation::CacheNode *node);

    protected:
      /* \brief Code to create the actor.
       *
       */
      void run();

    private:
      double                              m_brightness;
      double                              m_contrast;
      QColor                              m_color;
      DefaultVolumetricDataSPtr           m_data;
      Nm                                  m_position;
      Plane                               m_plane;
      CachedRepresentation::CacheNode    *m_node;
  };

  using ChannelSliceCachedRepresentationTaskPtr  = ChannelSliceCachedRepresentationTask *;
  using ChannelSliceCachedRepresentationTaskSPtr = std::shared_ptr<ChannelSliceCachedRepresentationTask>;

  //-----------------------------------------------------------------------------
  class SegmentationSliceCachedRepresentationTask
  : public CachedRepresentationTask
  {
    public:
      /* \brief Default class constructor
       *
       */
      explicit SegmentationSliceCachedRepresentationTask(SchedulerSPtr scheduler);

      /* \brief Default class destructor
       *
       */
      virtual ~SegmentationSliceCachedRepresentationTask()
      {};

      /* \brief Sets task input values.
       *
       *  The values must be valid to guarantee that an actor will be generated.
       */
      void setInput(DefaultVolumetricDataSPtr data, Nm position, Plane plane, double brightness, double contrast, QColor color, NmVector3 depth, CachedRepresentation::CacheNode *node);

    protected:
      /* \brief Code to create the actor.
       *
       */
      void run();

    private:
      QColor                           m_color;
      NmVector3                        m_depth;
      DefaultVolumetricDataSPtr        m_data;
      Nm                               m_position;
      Plane                            m_plane;
      CachedRepresentation::CacheNode *m_node;
  };

  using SegmentationSliceCachedRepresentationTaskPtr  = SegmentationSliceCachedRepresentationTask *;
  using SegmentationSliceCachedRepresentationTaskSPtr = std::shared_ptr<SegmentationSliceCachedRepresentationTask>;


} // namespace EspINA

#endif // ESPINA_SLICE_CACHED_REPRESENTATION_TASK_H_
