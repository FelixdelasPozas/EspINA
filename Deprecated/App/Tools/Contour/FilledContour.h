/*
 
 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef FILLEDCONTOUR_H
#define FILLEDCONTOUR_H

// ESPINA
#include <GUI/Tools/ITool.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>
#include <Tools/Brushes/Brush.h>
#include <GUI/vtkWidgets/ContourWidget.h>

class QUndoStack;
class vtkPolyData;

namespace ESPINA
{
  class ContourSelector;
  class ViewManager;
  class ContourWidget;

  class FilledContour
  : public ITool  // TODO Change to IROI to use countour as ROI
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

      // called by unrasterized UndoCommands
      void setContour(ContourWidget::ContourData contour);
      ContourWidget::ContourData getContour();

    signals:
      void changeMode(Brush::BrushMode);
      void stopDrawing();
      void startDrawing();

    public slots:
      void createUndoCommand();

    protected slots:
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

} // namespace ESPINA

#endif // FILLEDCONTOUR_H