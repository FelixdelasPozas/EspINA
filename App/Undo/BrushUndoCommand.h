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


#ifndef BRUSHUNDOCOMMAND_H
#define BRUSHUNDOCOMMAND_H

#include <QUndoCommand>

#include <Tools/Brushes/Brush.h>

class vtkImplicitFunction;

namespace EspINA
{
  class ViewManager;

  class Brush::DrawCommand
  : public QObject
  , public QUndoCommand
  {
    Q_OBJECT
  public:
    explicit DrawCommand(SegmentationSPtr seg,
                         BrushShapeList brushes,
                         itkVolumeType::PixelType value,
                         ViewManager *vm,
                         Brush *parent);
    virtual void redo();
    virtual void undo();

  signals:
    void initBrushTool();


  private:
    SegmentationSPtr m_seg;
    Filter::OutputId m_output;
    BrushShapeList   m_brushes;
    ViewManager     *m_viewManager;

    double m_strokeBounds[6];

    itkVolumeType::PixelType m_value;
    itkVolumeType::Pointer m_prevVolume;
    itkVolumeType::Pointer m_newVolume;
  };

  class Brush::SnapshotCommand
  : public QUndoCommand
  {
  public:
    explicit SnapshotCommand(SegmentationSPtr seg,
                             Filter::OutputId output,
                             ViewManager *vm);

    virtual void redo();
    virtual void undo();

  private:
    SegmentationSPtr m_seg;
    Filter::OutputId m_output;
    ViewManager     *m_viewManager;

    itkVolumeType::Pointer m_prevVolume;
    itkVolumeType::Pointer m_newVolume;
  };

} // namespace EspINA

#endif // BRUSHUNDOCOMMAND_H
