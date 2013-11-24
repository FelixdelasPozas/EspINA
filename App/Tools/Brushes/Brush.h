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

#include "App/Toolbars/Editor/Settings.h"
#include <Support/ITool.h>

class QUndoStack;

namespace EspINA
{
  class BrushSelector;
  class ViewManager;
  class VolumeSnapshotCommand;
  class EditorToolBarSettings;

  class Brush // TODO 2012-11-27 Crear una clase base para pintar independientemente de lo que se haga con el resultado
  : public Tool
  {
    Q_OBJECT
    enum DrawMode {CREATE, MODIFY};
  public:
    enum BrushMode {BRUSH, ERASER};

    typedef QPair<vtkImplicitFunction*, EspinaRegion> BrushShape;
    typedef QList<BrushShape> BrushShapeList;

    class DrawCommand;
    class SnapshotCommand;

    static const Filter::Type FREEFORM_SOURCE_TYPE;

  public:
    explicit Brush(EspinaModel *model,
                   EditorToolBarSettings *settins,
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
                            ISelector::WorldRegion centers,
                            Nm                   radius,
                            PlaneType            plane);

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
    BrushSelector *m_brush;
    EditorToolBarSettings *m_settings;

    FilterSPtr       m_currentSource;
    SegmentationSPtr m_currentSeg;
    FilterOutputId   m_currentOutput;
  };

} // namespace EspINA

#endif // BRUSH_H
