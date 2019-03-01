/*
 * Copyright (C) 2017, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef APP_TOOLGROUPS_EDIT_SLICEINTERPOLATIONTOOL_H_
#define APP_TOOLGROUPS_EDIT_SLICEINTERPOLATIONTOOL_H_

// ESPINA
#include <Support/Widgets/EditTool.h>
#include <Filters/SliceInterpolationFilter.h>

class QPushButton;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class NumericalInput;
      class ToolButton;
    }
  }
  /** \class SliceInterpolationTool
   * \brief Tool for slice interpolation filter.
   *
   */
  class SliceInterpolationTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief SliceInterpolation class constructor.
       * \param[in] context application context.
       *
       */
      SliceInterpolationTool(Support::Context &context);

      /** \brief SliceInterpolation class virtual destructor.
       *
       */
      virtual ~SliceInterpolationTool();

      virtual void abortOperation() override;
      virtual void restoreSettings(std::shared_ptr<QSettings> settings);
      virtual void saveSettings(std::shared_ptr<QSettings> settings);

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

      /** \brief Helper method that creates the tool parameters' widgets.
       *
       */
      void initSettingsWidgets();

    private slots:
      /** \brief Launches the operation task.
       *
       */
      void applyFilter();

      /** \brief Performs post operations after the task has finished.
       *
       */
      void onTaskFinished();

    private:
      /** \brief Stops and destroys all currently running tasks.
       *
       */
      void abortTasks();

      /** \struct TaskContext
       * \brief Context of currently executing tasks.
       *
       */
      struct TaskContext
      {
        SliceInterpolationFilterSPtr filter;       /** filter being executed. */
        SegmentationAdapterPtr       segmentation; /** input segmentation.    */

        /** \brief TaskContext stuct constructor.
         *
         */
        TaskContext(): filter{nullptr}, segmentation{nullptr} {};

        /** \brief TaskContext destructor.
         *
         */
        ~TaskContext()
        {
          filter = nullptr;
          segmentation = nullptr;
        }
      };

      GUI::Widgets::NumericalInput *m_threshold;      /** threshold value widget.                       */
      GUI::Widgets::ToolButton     *m_slicButton;     /** SLIC checkbox widget.                         */
      QMap<TaskPtr, TaskContext>    m_executingTasks; /** map of task<->context currently in execution. */
  };
} /* namespace ESPINA */

#endif /* APP_TOOLGROUPS_EDIT_SLICEINTERPOLATIONTOOL_H_ */
