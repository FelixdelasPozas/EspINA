/*
* Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
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

#ifndef APP_TOOLGROUPS_EDIT_FILLHOLES2DTOOL_H_
#define APP_TOOLGROUPS_EDIT_FILLHOLES2DTOOL_H_

// ESPINA
#include <Filters/FillHoles2DFilter.h>
#include <Support/Widgets/EditTool.h>

// QT
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace ESPINA
{
  class FillHoles2DTool
      : public Support::Widgets::EditTool
  {
    Q_OBJECT
    public:
      /** \brief FillHoles2DTool class constructor.
       * \param[in] context application context.
       *
       */
      FillHoles2DTool(Support::Context &context);

      /** \brief FillHoles2DTool class virtual destructor.
       *
       */
      virtual ~FillHoles2DTool();

      virtual void abortOperation() override;

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

      /** \brief Initializes the split option widgets.
       *
       */
      void initOptionWidgets();

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
      struct TaskContext
      {
          FillHoles2DFilterSPtr Filter;        /** filter being executed. */
          SegmentationAdapterPtr Segmentation; /** segmentation.          */
      };

      QMap<TaskPtr, TaskContext> m_executingTasks; /** map of task<->context currently in execution. */

      QLabel      *m_directionLabel;    /** label containing "Orthogonal Direction" text. */
      QComboBox   *m_directionComboBox; /** comboBox selector with direction options.     */
      QPushButton *m_applyButton;       /** apply filter button.                          */
  };
}

#endif /* APP_TOOLGROUPS_EDIT_FILLHOLES2DTOOL_H_ */
