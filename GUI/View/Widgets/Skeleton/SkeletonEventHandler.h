/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_VIEW_WIDGETS_SKELETON_SKELETONEVENTHANDLER_H_
#define GUI_VIEW_WIDGETS_SKELETON_SKELETONEVENTHANDLER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <GUI/View/EventHandler.h>

// Qt
#include <QList>
#include <QPoint>

// VTK
#include <vtkSmartPointer.h>

class vtkCoordinate;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Skeleton
        {
          /** \class SkeletonEventHandler
           * \brief Handles view events for skeleton tools.
           *
           */
          class SkeletonEventHandler
          : public EventHandler
          {
              Q_OBJECT
            public:
              using Track = QList<QPoint>;

              /** \brief SkeletonEventHandler class constructor.
               *
               */
              explicit SkeletonEventHandler();

              /** \brief SkeletonEventHandler class virtual destructor.
               *
               */
              virtual ~SkeletonEventHandler()
              {}

              virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

              /** \brief Enables/Disables point interpolation.
               * \param[in] active true to enable false otherwise.
               *
               */
              void setInterpolation(bool active);

              /** \brief Returns true if the point interpolation is enabled and false otherwise.
               *
               */
              bool interpolationEnabled() const
              { return m_interpolation; }

              /** \brief Sets the maximum distance between two consecutive track points. If this distance
               * is not met when building the track then the handler will add more points to meet
               * this requirement.
               * \param[in] distance maximum distance between two consecutive track points.
               *
               */
              void setMaximumPointDistance(const Nm distance);

              /** \brief Returns the maximum distance between two consecutive points of the tracks the
               * handler will produce.
               *
               */
              Nm maximumPointDistance() const
              { return std::sqrt(m_maxDistance2); }

              /** \brief Sets the minimum distance between two consecutive track points. If this distance
               * is not met when building the track then the handler won't add the point.
               * \param[in] distance minimum distance between two consecutive track points.
               *
               */
              void setMinimumPointDistance(const Nm distance);

              /** \brief Returns the minimum distance between two consecutive points of the track the handler
               * will produce.
               *
               */
              const Nm minimumPointDistance() const
              { return std::sqrt(m_minDistance2); }

              /** \brief Returns true if the handler is in the middle of a track.
               *
               */
              bool isTracking() const;

            signals:
              void trackStarted(Track track, RenderView *view);
              void trackUpdated(Track track);
              void trackStopped(Track track, RenderView *view);
              void cursorPosition(const QPoint &p);
              void cancelled();
              void endStroke();

            protected:
              /** \brief Called when a stroke starts.
               * \param[in] pos track point 2D position.
               *
               */
              void startTrack(const QPoint &pos);

              /** \brief Called when a stroke is updated.
               * \param[in] pos track point 2D position.
               *
               */
              void updateTrack(const QPoint &pos);

              /** \brief Called when a stroke ends.
               * \param[in] pos track point 2D position.
               *
               */
              void stopTrack(const QPoint &pos);

              /** \brief Interpolates p1 and p2 and generates extra points using the max distance.
               * \param[in] p1 point 3D coordinates.
               * \param[in] p2 point 3D coordinates.
               *
               */
              Track interpolate(const QPoint &p1, const QPoint &p2);

              /** \brief Returns the squared distance between the two given points.
               * \param[in] p1 point 3D coordinates.
               * \param[in] p2 point 3D coordinates.
               *
               */
              Nm distance2(const NmVector3 &p1, const NmVector3 &p2);

              bool  m_tracking;            /** true if tracking and false otherwise.                      */
              bool  m_interpolation;       /** true if interpolation is being made.                       */
              Nm    m_maxDistance2;        /** max distance between points of the track (in view coords). */
              Nm    m_minDistance2;        /** min distance between points of the track (in view coords). */
              bool  m_distanceHasBeenSet;  /** true if max distance has been set.                         */
              Track m_track;               /** track points group.                                        */
              Track m_updatedTrack;        /** group of points in the update track step.                  */

              RenderView *m_view; /** view under event handler. */
          };

          using SkeletonEventHandlerSPtr = std::shared_ptr<SkeletonEventHandler>;
        
        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_SKELETON_SKELETONEVENTHANDLER_H_
