/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  <copyright holder> <email>

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

#ifndef FILLEDCONTOUR_H
#define FILLEDCONTOUR_H

// EspINA
#include <GUI/Tools/ITool.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>
#include <Tools/Brushes/Brush.h>
#include <GUI/vtkWidgets/ContourWidget.h>

class QUndoStack;
class vtkPolyData;

namespace EspINA
{
  class ContourSelector;
  class ViewManager;
  class ContourWidget;

  class FilledContour
  : public ITool  // TODO Change to IVOI to use countour as VOI
  {
    Q_OBJECT
    public:
      static const Filter::FilterType FILTER_TYPE;

    public:
      explicit FilledContour(EspinaModel *model, QUndoStack *undoStack, ViewManager *viewManager);
      virtual ~FilledContour();

      virtual QCursor cursor() const;
      virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
      virtual void setInUse(bool enable);
      virtual void setEnabled(bool enable);
      virtual bool enabled() const;

      // only called from undoCommands
      virtual void abortOperation();

    signals:
      void changeMode(Brush::BrushMode);
      void stopDrawing();

    protected slots:
      void storeContourData();
      void rasterize(ContourWidget::ContourList);

    private:
      EspinaModel *m_model;
      QUndoStack *m_undoStack;
      ViewManager *m_viewManager;

      ContourSelector *m_picker;
      bool m_enabled;
      bool m_inUse;

      ContourWidget *m_contourWidget;

      FilterSPtr m_currentSource;
      SegmentationSPtr m_currentSeg;

      bool m_widgetHasContour;
      vtkPolyData *m_lastContour;
  };

  typedef boost::shared_ptr<FilledContour> FilledContourSPtr;

} // namespace EspINA

#endif // FILLEDCONTOUR_H
