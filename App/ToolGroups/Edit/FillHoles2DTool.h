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

#include <Support/Widgets/EditTool.h>

namespace ESPINA
{
	class FillHoles2DTool
	: public Support::Widgets::EditTool
	{
		Q_OBJECT
		public:
		FillHoles2DTool(Support::Context &context);

		virtual ~FillHoles2DTool(){};

		virtual void abortOperation();

		private:
		virtual bool acceptsNInputs(int n) const;

		virtual bool acceptsSelection(SegmentationAdapterList segmentations);

		private slots:
		void fillHoles2D();

		void onTaskFinished();

		private:
		struct TaskContext
		{
		FilterSPtr             Task;
		SegmentationAdapterPtr Segmentation;
		QString                Operation;
		};
		QMap<TaskPtr, TaskContext> m_executingTasks;
	};
}

#endif /* APP_TOOLGROUPS_EDIT_FILLHOLES2DTOOL_H_ */
