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
          class SkeletonWidget2D;

          /** \class SkeletonEventHandler
           * \brief Handles view events for skeleton tools.
           *
           */
          class EspinaGUI_EXPORT SkeletonEventHandler
          : public EventHandler
          {
              Q_OBJECT
            public:
              using Track = QList<QPoint>;
              enum Mode: char { CREATE = 0, OTHER = 1 };

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
              const Nm maximumPointDistance() const
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

              /** \brief Adds a widget to observe to the list.
               *
               */
              void addWidget(SkeletonWidget2D *widget);

              /** \brief Removes a widget from the list of widgets to observe.
               *
               */
              void removeWidget(SkeletonWidget2D *widget);

              /** \brief Sets the operation mode.
               * \param[in] mode new operation mode.
               *
               */
              void setMode(Mode mode);

              /** \brief Returns the current operation mode.
               *
               */
              Mode mode() const
              { return m_mode; }

              /** \brief Sets the event handler to respond to modifier keyboard events or ignore them.
               * \param[in] value True to respond to keyboard events and false to ignore them.
               *
               */
              void setAllowModifiers(const bool value);

              /** \brief Returns true if the handler responds to modifiers events or false if it does not.
               *
               */
              const bool allowsModifiers() const
              { return m_allowModifiers; }

            public slots:
              /** \brief Updates the widget's representations.
               *
               */
              void updateRepresentations();

            signals:
              void started(Track, RenderView *);
              void updated(Track, RenderView *);
              void cursorPosition(const QPoint, RenderView *);
              void cancelled(RenderView *);
              void stopped(RenderView *);
              void modifier(bool);
              void mousePress(Qt::MouseButtons, const QPoint, RenderView *);
              void mouseRelease(Qt::MouseButtons, const QPoint, RenderView *);

            protected:
              /** \brief Called when a stroke starts.
               * \param[in] pos track point 2D position.
               * \param[in] view view of the event.
               *
               */
              void startTrack(const QPoint &pos, RenderView *view);

              /** \brief Called when a stroke is updated.
               * \param[in] pos track point 2D position.
               * \param[in] view view of the event.
               *
               */
              void updateTrack(const QPoint &pos, RenderView *view);

              /** \brief Interpolates p1 and p2 and generates extra points using the max distance.
               * \param[in] point1 point 3D coordinates.
               * \param[in] point2 point 3D coordinates.
               * \param[in] view view of the event.
               *
               */
              Track interpolate(const QPoint &point1, const QPoint &point2, RenderView *view);

              /** \brief Returns the squared distance between the two given points.
               * \param[in] point1 point 3D coordinates.
               * \param[in] point2 point 3D coordinates.
               *
               */
              inline Nm distance2(const NmVector3 &point1, const NmVector3 &point2);

              bool        m_tracking;           /** true if tracking and false otherwise.                                */
              bool        m_interpolation;      /** true if interpolation is being made.                                 */
              Nm          m_maxDistance2;       /** max distance between points of the track (in view coords).           */
              Nm          m_minDistance2;       /** min distance between points of the track (in view coords).           */
              bool        m_distanceHasBeenSet; /** true if max distance has been set.                                   */
              Track       m_track;              /** track points group.                                                  */
              Track       m_updatedTrack;       /** group of points in the update track step.                            */
              Mode        m_mode;               /** current operation mode.                                              */
              RenderView *m_view;               /** current operation view, only valid in CREATE mode.                   */
              bool        m_allowModifiers;     /** true to allow modifiers (shift key events) and false to ignore them. */

              QList<SkeletonWidget2D *> m_widgets; /** list of widgets to observe events.                    */
          };

          using SkeletonEventHandlerSPtr = std::shared_ptr<SkeletonEventHandler>;
        
        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_SKELETON_SKELETONEVENTHANDLER_H_
