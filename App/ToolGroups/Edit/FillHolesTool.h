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

#ifndef ESPINA_FILL_HOLES_TOOL_H
#define ESPINA_FILL_HOLES_TOOL_H

// ESPINA
#include <Filters/FillHolesFilter.h>
#include <Support/Widgets/EditTool.h>

namespace ESPINA
{
  /** \class FillHolesTool
   * \brief Tool for fill holes filter.
   *
   */
  class FillHolesTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief FillHolesTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit FillHolesTool(Support::Context &context);

      /** \brief FillHolesTool class virtual destructor.
       *
       */
      virtual ~FillHolesTool()
      {};

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
        FillHolesFilterSPtr    Filter;       /** filter in execution. */
        SegmentationAdapterPtr Segmentation; /** segmentation.        */
      };

      QMap<TaskPtr, TaskContext> m_executingTasks; /** map filter<->context of currently executing filters. */
  };
}

#endif // ESPINA_FILLHOLESTOOL_H
