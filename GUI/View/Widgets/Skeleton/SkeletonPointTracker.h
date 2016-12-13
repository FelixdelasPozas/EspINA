/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_VIEW_WIDGETS_SKELETON_SKELETONPOINTTRACKER_H_
#define GUI_VIEW_WIDGETS_SKELETON_SKELETONPOINTTRACKER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/PointTracker.h>

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
          /** \class SkeletonPointTracker
           * \brief PointTracker with the interaction needed by the Skeleton drawing tool.
           *
           */
          class EspinaGUI_EXPORT SkeletonPointTracker
          : public PointTracker
          {
              Q_OBJECT
            public:
              /** \brief SkeletonPointTracker class constructor.
               *
               */
              explicit SkeletonPointTracker()
              {};

              /** \brief SkeletonPointTracker class virtual destructor.
               *
               */
              virtual ~SkeletonPointTracker()
              {};

              virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

            signals:
              void endStroke();

              void cursorPosition(const QPoint &p);
          };

          using SkeletonPointTrackerPtr  = SkeletonPointTracker *;
          using SkeletonPointTrackerSPtr = std::shared_ptr<SkeletonPointTracker>;

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_SKELETON_SKELETONPOINTTRACKER_H_
