/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_IMAGE_LOGIC_TOOL_H
#define ESPINA_IMAGE_LOGIC_TOOL_H

#include <Support/Widgets/EditTool.h>
#include <Filters/ImageLogicFilter.h>

namespace ESPINA
{
  /** \class ImageLogicTool
   * \brief Tool for logical operation filters.
   *
   */
  class ImageLogicTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief ImageLogicTool class constructor.
       * \param[in] id tool id.
       * \param[in] icon icon string in the aplication resource file.
       * \param[in] tooltip tool tooltip text.
       * \param[in] context application context.
       *
       */
      explicit ImageLogicTool(const QString &id, const QString &icon, const QString &tooltip, Support::Context &context);

      /** \brief ImageLogicTool class virtual destructor.
       *
       */
      virtual ~ImageLogicTool();

      /** \brief Sets the logic operation of the tool
       * \param[in] operation logic operation of the tool.
       *
       */
      void setOperation(ImageLogicFilter::Operation operation);

      /** \brief Sets if the segmentations of a substract operation should be removed after finishing.
       * \param[in] value true to remove and false otherwise.
       *
       * NOTE: this has no effect on addition operation, obviously.
       */
      void removeOnSubtract(bool value)
      { m_removeOnSubtract = value; }

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations);

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
      /** \brief Aborts the currently executing tasks.
       *
       */
      void abortTasks();

      /** context of the operation task. */
      struct TaskContext
      {
        FilterSPtr                  Task;           /** task object.                                                   */
        ImageLogicFilter::Operation Operation;      /** operation of the task.                                         */
        SegmentationAdapterList     Segmentations;  /** list of segmentation involved in the operation.                */
        bool                        Remove;         /** true to remove the segmentations after the operation finishes. */
      };

      ImageLogicFilter::Operation m_operation;         /** operation of the tool.                 */
      bool                        m_removeOnSubtract;  /** optional flag for subtract operation.  */
      QMap<TaskPtr, TaskContext>  m_executingTasks;    /** map of task-context for running tasks. */
  };
}

#endif // ESPINA_IMAGE_LOGIC_TOOL_H
