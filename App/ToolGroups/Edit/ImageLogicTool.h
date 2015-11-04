/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_IMAGE_LOGIC_TOOL_H
#define ESPINA_IMAGE_LOGIC_TOOL_H

#include <Support/Widgets/EditTool.h>
#include <Filters/ImageLogicFilter.h>

namespace ESPINA
{
  class ImageLogicTool
  : public Support::Widgets::EditTool
  {
    Q_OBJECT
  public:
    explicit ImageLogicTool(const QString &id, const QString &icon, const QString &tooltip, Support::Context &context);

    void setOperation(ImageLogicFilter::Operation operation);

    virtual void abortOperation();

  private:
    virtual bool acceptsNInputs(int n) const;

    virtual bool acceptsSelection(SegmentationAdapterList segmentations);

  private slots:
    void applyFilter();

    void onTaskFinished();

  private:
    struct TaskContext
    {
      FilterSPtr                  Task;
      ImageLogicFilter::Operation Operation;
      SegmentationAdapterList     Segmentations;
      bool                        Remove;
    };

    ImageLogicFilter::Operation m_operation;

    QMap<TaskPtr, TaskContext> m_executingTasks;
  };
}

#endif // ESPINA_IMAGE_LOGIC_TOOL_H
