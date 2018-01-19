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

#ifndef APP_TOOLGROUPS_ANALYZE_SKELETONINSPECTOR_SKELETONINSPECTORTOOL_H_
#define APP_TOOLGROUPS_ANALYZE_SKELETONINSPECTOR_SKELETONINSPECTORTOOL_H_

// ESPINA
#include <GUI/Types.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  class SkeletonInspector;

  /** \class SkeletonInspectorTool
   * \brief Skeleton inspector dialog tool button.
   *
   */
  class SkeletonInspectorTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief SkeletonInspectorTool class constructor.
       * \param[in] context Application context.
       *
       */
      explicit SkeletonInspectorTool(Support::Context &context);

      /** \brief SkeletonInspectorTool class virtual destructor.
       *
       */
      virtual ~SkeletonInspectorTool()
      {}

    private slots:
      /** \brief Displays the SkeletonInspector dialog.
       * \param[in] unused
       *
       */
      void onPressed(bool unused);

      /** \brief Enables/Disables the tool depending on the current segmentation selection.
       *
       */
      void onSelectionChanged(SegmentationAdapterList selectedSegs);

    private:
      /** \brief Helper method to connect QObject's signals.
       *
       */
      void connectSignals();
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_ANALYZE_SKELETONINSPECTOR_SKELETONINSPECTORTOOL_H_
