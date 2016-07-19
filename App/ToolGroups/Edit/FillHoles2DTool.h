/*
 * FillHoles2DTool.h
 *
 *  Created on: 19 de jul. de 2016
 *      Author: heavy
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
