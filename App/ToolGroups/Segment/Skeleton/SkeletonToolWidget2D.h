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

#ifndef APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLWIDGET2D_H_
#define APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLWIDGET2D_H_

// ESPINA
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolsEventHandler.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

namespace ESPINA
{
  /** \class SkeletonToolWidget2D
   * \brief Specialized widget for skeleton tools in EspINA.
   *
   */
  class SkeletonToolWidget2D
  : public GUI::View::Widgets::Skeleton::SkeletonWidget2D
  {
      Q_OBJECT
    public:
      /** \brief SkeletonToolWidget2D class constructor.
       * \param[in] handler SkeletonHandler for this widget.
       * \param[in] settings Skeleton representation settings.
       */
      explicit SkeletonToolWidget2D(SkeletonToolsEventHandlerSPtr handler, GUI::Representations::Settings::SkeletonPoolSettingsSPtr settings);

      /** \brief SkeletonToolWidget2D class virtual destructor.
       *
       */
      virtual ~SkeletonToolWidget2D()
      {};

      /** \brief Returns true if the given point can be the start of a stroke and does not connect with any other point of a stroke.
       * \param[in] point Point 3d coodinates.
       *
       */
      bool isStartNode(const NmVector3 &point);

    public slots:
      /** \brief Modifies the current skeleton to build a connection of the given stroke.
       * \param[in] category Stroke category in the strokes list.
       * \param[in] strokeIndex Index in the stroke list for building the connection.
       * \param[in] plane Plane of the event.
       *
       */
      void onConnectionSignaled(const QString &category, const int strokeIndex, const Plane plane);

      /** \brief Modifies the current skeleton to change stroke type after the next point.
       * \param[in] category Stroke category in the strokes list.
       * \param[in] strokeIndex Index in the stroke list for stroke to change to.
       * \param[in] plane Plane of the event.
       *
       */
      void onStrokeChangeSignaled(const QString &category, const int strokeIndex, const Plane plane);

      virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation() override;

    private:
      SkeletonToolsEventHandlerSPtr                            m_toolHandler; /** handler for this tool.   */
      GUI::Representations::Settings::SkeletonPoolSettingsSPtr m_settings;    /** representation settings. */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_SEGMENT_SKELETON_SKELETONTOOLWIDGET2D_H_
