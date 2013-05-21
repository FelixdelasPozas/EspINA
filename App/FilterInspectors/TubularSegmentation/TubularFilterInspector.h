/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef TUBULARFILTERINSPECTOR_H
#define TUBULARFILTERINSPECTOR_H

#include "ui_TubularFilterInspector.h"

// EspINA
#include <Core/Filters/TubularSegmentationFilter.h>
#include <GUI/Pickers/PixelSelector.h>
#include <GUI/Tools/ITool.h>

// Qt
#include <QUndoStack>
#include <QVector4D>

class QUndoStack;

namespace EspINA
{
  class ViewManager;
  class TubularTool;

  class TubularFilterInspector
  : public Filter::FilterInspector
  {
    public:
      explicit TubularFilterInspector(TubularSegmentationFilter::Pointer source,
                                      QUndoStack *undo,
                                      ViewManager *vm,
                                      IToolSPtr tool);
      virtual ~TubularFilterInspector();

      virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager);

      class Widget;
    private:
      TubularSegmentationFilter::Pointer m_source;
      QUndoStack                        *m_undoStack;
      ViewManager                       *m_viewManager;
      IToolSPtr                          m_tool;
  };

  class TubularFilterInspector::Widget
  : public QWidget
  , public Ui::TubularFilterInspector
  {
    Q_OBJECT
    public:
      explicit Widget(TubularSegmentationFilter::Pointer source,
                      QUndoStack *undo,
                      ViewManager *vm,
                      IToolSPtr tool);
      virtual ~Widget();

    protected slots:
      void exportNodeList();
      void updateNodeList();
      void updateSpine(TubularSegmentationFilter::NodeList nodes);
      void editSpine(bool editing);
      void modifyRoundedExtremes(bool);

    private:
      TubularSegmentationFilter::Pointer m_source;
      QUndoStack                        *m_undoStack;
      ViewManager                       *m_viewManager;
      IToolSPtr                          m_tool;
  };
}
#endif // TUBULARFILTERINSPECTOR_H
