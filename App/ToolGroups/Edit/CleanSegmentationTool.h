/*
 File: CleanSegmentationTool.h
 Created on: 25/07/2019
 Author: Felix de las Pozas Alvarez

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

#ifndef APP_TOOLGROUPS_EDIT_CLEANSEGMENTATIONTOOL_H_
#define APP_TOOLGROUPS_EDIT_CLEANSEGMENTATIONTOOL_H_

// Project
#include <Filters/CleanSegmentationVoxelsFilter.h>
#include <Support/Widgets/EditTool.h>

namespace ESPINA
{
  /** \class CleanSegmentationTool
   * \brief Implements the tool part of the CleanSegmentationFilter filter.
   *
   */
  class CleanSegmentationTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief CleanSegmentationTool class constructor.
       * \param[in] context Application context.
       *
       */
      explicit CleanSegmentationTool(Support::Context &context);

      /** \brief CleanSegmentationTool class virtual destructor.
       *
       */
      virtual ~CleanSegmentationTool()
      {}

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

    private slots:
      /** \brief Lauches task execution.
       *
       */
      void applyFilter();

      /** \brief Processes execution results.
       *
       */
      void onTaskFinished();

    private:
      /** \brief Aborts all executing tasks.
       *
       */
      void abortTasks();

      /** \struct TaskContext
       * \brief Executing task context.
       *
       */
      struct TaskContext
      {
        CleanSegmentationVoxelsFilterSPtr Filter;       /** Filter.              */
        SegmentationAdapterPtr            Segmentation; /** segmentation.        */
      };

      QMap<TaskPtr, TaskContext> m_executingTasks; /** map filter<->context of currently executing filters. */

  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_EDIT_CLEANSEGMENTATIONTOOL_H_
