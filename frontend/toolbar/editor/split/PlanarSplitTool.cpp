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


#include "PlanarSplitTool.h"
#include "SplitUndoCommand.h"

#include <editor/split/SplitFilter.h>
#include <gui/ViewManager.h>
#include <model/Segmentation.h>
#include <model/EspinaModel.h>
#include <model/EspinaFactory.h>
#include <undo/AddSegmentation.h>
#include <QUndoStack>

//-----------------------------------------------------------------------------
PlanarSplitTool::PlanarSplitTool(EspinaModel* model,
                                 QUndoStack* undoStack,
                                 ViewManager* viewManager)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_inUse(false)
, m_enable(true)
, m_widget(NULL)
{

}

//-----------------------------------------------------------------------------
QCursor PlanarSplitTool::cursor() const
{
  return QCursor(Qt::ArrowCursor);
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (m_inUse && m_enable && m_widget)
    return m_widget;

  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitTool::enabled() const
{
  return m_enable;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setEnabled(bool value)
{
  if (m_enable != value)
    m_enable = value;
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::setInUse(bool value)
{
  if (m_inUse != value)
  {
    m_inUse = value;
    if (!m_inUse)
      emit splittingStopped();
    else //Tmp
      splitSegmentation();
  }
}

//-----------------------------------------------------------------------------
void PlanarSplitTool::splitSegmentation()
{
  Filter::NamedInputs inputs;
  Filter::Arguments   args;

  SegmentationList selectedSegs = m_viewManager->selectedSegmentations();
  Q_ASSERT(selectedSegs.size() == 1);

  Segmentation *seg = selectedSegs.first();

  inputs[SplitFilter::INPUTLINK] = seg->filter();
  args[Filter::INPUTS] = args.namedInput(SplitFilter::INPUTLINK, seg->outputNumber());

  SplitFilter *filter = new SplitFilter(inputs, args);
  filter->update();

  if (filter->numberOutputs() == 2)
    m_undoStack->push(new SplitUndoCommand(seg, filter, m_model));
  //else
    //report message
}
