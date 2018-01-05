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

#ifndef SKELETONTOOLSEVENTHANDLER_H_
#define SKELETONTOOLSEVENTHANDLER_H_


// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Types.h>
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Support/Context.h>

// VTK
#include <vtkSmartPointer.h>

class QEvent;
class QAction;
class QMenu;
class vtkPolyData;

namespace ESPINA
{
  /** \class SkeletonEventHandler
   * \brief Handles view events for skeleton tools.
   *
   */
  class SkeletonToolsEventHandler
  : public GUI::View::Widgets::Skeleton::SkeletonEventHandler
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief SkeletonEventHandler class constructor.
       * \param[in] context Application context.
       *
       */
      explicit SkeletonToolsEventHandler(Support::Context &context);

      /** \brief SkeletonEventHandler class virtual destructor.
       *
       */
      virtual ~SkeletonToolsEventHandler();

      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;

      /** \brief Sets the category to get the strokes definition.
       * \param[in] category Category name.
       *
       */
      void setStrokesCategory(const QString &category);

    signals:
      void selectedStroke(int index);
      void addConnectionPoint(const NmVector3 point);
      void removeConnectionPoint(const NmVector3 point);
      void clearConnections();
      void addEntryPoint(const NmVector3 point);
      void signalConnection(const QString category, const int strokeIndex, const Plane plane);

    private slots:
      /** \brief Signals the selection of a given stroke definition.
       *
       */
      void onActionSelected(QAction *action);

      /** \brief Hides the menu if shown in case of deactivation.
       * \param[in] enabled true if handler has been enabled and false in case it has been disabled.
       *
       */
      void onHandlerUseChanged(bool enabled);

    protected:
      using PointTemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

      QMenu                       *m_contextMenu;    /** strokes context menu.                                                  */
      QMenu                       *m_connectionMenu; /** Connection type context menu.                                          */
      QString                      m_category;       /** current segmentation category.                                         */
      bool                         m_checkCollision; /** true to check for point collision with segmentations, false otherwise. */
      bool                         m_cancelled;      /** true if the menu was cancelled, false otherwise.                       */
      Extensions::ChannelEdgesSPtr m_edges;          /** active stack edges extension.                                          */
      Plane                        m_plane;          /** plane of the last event.                                               */
      NmVector3                    m_point;          /** last connection point.                                                 */
  };

  using SkeletonToolsEventHandlerSPtr = std::shared_ptr<SkeletonToolsEventHandler>;

} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_SKELETON_SKELETONEVENTHANDLER_H_
