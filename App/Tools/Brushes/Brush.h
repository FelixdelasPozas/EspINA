/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef BRUSH_H
#define BRUSH_H

#include <Core/EspinaRegion.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include "GUI/Pickers/IPicker.h"
#include "GUI/Tools/ITool.h"
#include "GUI/ViewManager.h"

class QUndoStack;

namespace EspINA
{
  class ViewManager;
  class BrushPicker;

  class Brush // TODO 2012-11-27 Crear una clase base para pintar independientemente de lo que se haga con el resultado
  : public ITool
  {
    Q_OBJECT

    enum DrawMode {CREATE, MODIFY};

  public:
    enum BrushMode {BRUSH, ERASER};

    typedef QPair<vtkImplicitFunction *, EspinaRegion> BrushShape;
    typedef QList<BrushShape> BrushShapeList;

    class DrawCommand;
    class SnapshotCommand;

    static const Filter::FilterType FREEFORM_SOURCE_TYPE;

  public:
    explicit Brush(EspinaModel *model,
                   QUndoStack  *undoStack,
                   ViewManager *viewManager);
    virtual ~Brush();

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
    virtual bool enabled() const;
    virtual void setEnabled(bool enable);
    virtual void setInUse(bool value);

  protected slots:
    virtual BrushShape createBrushShape(PickableItemPtr item,
                                        double          center[3],
                                        Nm              radius,
                                        PlaneType       plane)=0;

    virtual void drawStroke(PickableItemPtr      item,
                            IPicker::WorldRegion centers,
                            Nm                   radius,
                            PlaneType            plane);

    virtual void drawStrokeStep(PickableItemPtr item,
                                double x, double y, double z,
                                Nm              radius,
                                PlaneType       plane);
  virtual void segmentationHasBeenModified(ModelItemPtr);

  virtual void initBrushTool();

  signals:
    void brushModeChanged(Brush::BrushMode);
    void stopDrawing();

  protected:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    bool         m_inUse;
    DrawMode     m_mode;
    bool         m_erasing;
    BrushPicker *m_brush;

    FilterSPtr        m_currentSource;
    SegmentationSPtr  m_currentSeg;
    Filter::OutputId m_currentOutput;

  private:
    SnapshotCommand *m_drawCommand;
    SnapshotCommand *m_eraseCommand;
  };

} // namespace EspINA

#endif // BRUSH_H
