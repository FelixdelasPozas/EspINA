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

// ESPINA
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include "ROITools.h"
#include "CleanROITool.h"
#include "ManualROITool.h"
#include "OrthogonalROITool.h"
#include <Undo/ROIUndoCommand.h>

using namespace ESPINA;

class ROIToolsGroup::DefineOrthogonalROICommand
: public QUndoCommand
{
public:
  explicit DefineOrthogonalROICommand(ROISPtr roi, ROIToolsGroup *tool);

  virtual void redo() override;

  virtual void undo() override;

private:
  ROISPtr        m_ROI;
  ROIToolsGroup *m_tool;
  ROISPtr        m_prevROI;
};

//-----------------------------------------------------------------------------
ROIToolsGroup::DefineOrthogonalROICommand::DefineOrthogonalROICommand(ROISPtr roi, ROIToolsGroup* tool)
: m_ROI(roi)
, m_tool(tool)
, m_prevROI{tool->m_ortogonalROITool->currentROI()}
{
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::DefineOrthogonalROICommand::redo()
{
  m_tool->commitPendingOrthogonalROI(m_ROI);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::DefineOrthogonalROICommand::undo()
{
  m_tool->m_ortogonalROITool->setROI(m_prevROI);
}


//-----------------------------------------------------------------------------
class ROIToolsGroup::DefineManualROICommand
: public QUndoCommand
{
public:
  explicit DefineManualROICommand(const BinaryMaskSPtr<unsigned char> mask, ROIToolsGroup *tool);

  virtual void redo() override;

  virtual void undo() override;

private:
  const BinaryMaskSPtr<unsigned char> m_mask;
  ROIToolsGroup *m_tool;
  ROISPtr        m_oROI;
  bool           m_firstROI;
};

//-----------------------------------------------------------------------------
ROIToolsGroup::DefineManualROICommand::DefineManualROICommand(const BinaryMaskSPtr<unsigned char> mask, ROIToolsGroup* tool)
: m_mask{mask}
, m_tool{tool}
, m_oROI{tool->m_ortogonalROITool->currentROI()}
, m_firstROI{tool->m_accumulator == nullptr}
{
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::DefineManualROICommand::redo()
{
  m_tool->commitPendingOrthogonalROI(nullptr);
  m_tool->addMask(m_mask);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::DefineManualROICommand::undo()
{
  if (m_firstROI)
  {
    m_tool->setCurrentROI(m_oROI);
  }
  else
  {
    m_tool->m_accumulator->undo();

    if (m_oROI)
    {
      m_tool->m_accumulator->undo();
      m_tool->m_ortogonalROITool->setROI(m_oROI);
    }
  }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ROIToolsGroup::ROIToolsGroup(ROISettings*     settings,
                             ModelAdapterSPtr model,
                             ModelFactorySPtr factory,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack,
                             QWidget         *parent)
: ToolGroup          {viewManager, QIcon(":/espina/voi.svg"), tr("Volume Of Interest Tools"), parent}
, m_viewManager      {viewManager}
, m_undoStack        {undoStack}
, m_manualROITool    {new ManualROITool(model, viewManager, undoStack, this)}
, m_ortogonalROITool {new OrthogonalROITool(settings, model, viewManager, undoStack, this)}
, m_cleanROITool     {new CleanROITool(model, viewManager, undoStack, this)}
, m_enabled          {true}
, m_visible          {true}
, m_color            {Qt::yellow}
, m_accumulator      {nullptr}
, m_accumulatorWidget{nullptr}
{
  setColor(m_color);

  connect(m_manualROITool.get(),    SIGNAL(roiDefined(Selector::Selection)),
          this,                     SLOT(onManualROIDefined(Selector::Selection)));
  connect(m_ortogonalROITool.get(), SIGNAL(roiDefined(ROISPtr)),
          this,                     SLOT(onOrthogonalROIDefined(ROISPtr)));
}

//-----------------------------------------------------------------------------
ROIToolsGroup::~ROIToolsGroup()
{
  setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setEnabled(bool value)
{
  if(m_enabled == value)
    return;

  m_manualROITool   ->setEnabled(value);
  m_ortogonalROITool->setEnabled(value);
  m_cleanROITool    ->setEnabled(value);

  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool ROIToolsGroup::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList ROIToolsGroup::tools()
{
  ToolSList availableTools;

  availableTools << m_manualROITool;
  availableTools << m_ortogonalROITool;
  availableTools << m_cleanROITool;

  return availableTools;
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setCurrentROI(ROISPtr roi)
{
  if(m_accumulator)
  {
    m_viewManager->removeWidget(m_accumulatorWidget);

    m_accumulator      .reset();
    m_accumulatorWidget.reset();
  }

  if (roi && roi->isOrthogonal())
  {
    m_ortogonalROITool->setROI(roi);
  }
  else
  {
    m_ortogonalROITool->setROI(nullptr);
  }

  if (roi && !roi->isOrthogonal())
  {
    auto widget = std::make_shared<ROIWidget>(roi);
    widget->setColor(m_color);

    m_accumulator       = roi;
    m_accumulatorWidget = widget;

    m_viewManager->addWidget(m_accumulatorWidget);
  }
  m_viewManager->updateViews();

  emit roiChanged(roi);
}

//-----------------------------------------------------------------------------
ROISPtr ROIToolsGroup::currentROI()
{
  auto roi = m_ortogonalROITool->currentROI();

  if (m_accumulator)
  {
    if (roi)
    {
      commitPendingOrthogonalROI(nullptr);
    }
    roi = m_accumulator;
  }

  return roi;
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::consumeROI()
{
  // TODO: Must be undoable
  setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setColor(const QColor& color)
{
  m_color = color;

  m_manualROITool->setColor(color);
  m_ortogonalROITool->setColor(color);
}

//-----------------------------------------------------------------------------
bool ROIToolsGroup::hasValidROI() const
{
  return m_accumulator || m_ortogonalROITool->currentROI();
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::setVisible(bool visible)
{
  if (m_visible == visible) return;

  if (m_accumulator)
  {
    if (visible)
    {
      m_viewManager->addWidget(m_accumulatorWidget);
    }
    else
    {
      m_viewManager->removeWidget(m_accumulatorWidget);
    }
  }

  if (m_ortogonalROITool->currentROI())
  {
    m_ortogonalROITool->setVisible(visible);
  }

  m_visible = visible;
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::onManualROIDefined(Selector::Selection strokes)
{
  commitPendingOrthogonalROI(nullptr);

  if(hasValidROI())
  {
    m_undoStack->beginMacro("Modify Region Of Interest");
  }
  else
  {
    m_undoStack->beginMacro("Create Region Of Interest");
  }
  m_undoStack->push(new DefineManualROICommand{strokes.first().first, this});
  m_undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::onOrthogonalROIDefined(ROISPtr roi)
{
  if(hasValidROI())
  {
    m_undoStack->beginMacro("Modify Region Of Interest");
  }
  else
  {
    m_undoStack->beginMacro("Create Region Of Interest");
  }
  m_undoStack->push(new DefineOrthogonalROICommand{roi, this});
  m_undoStack->endMacro();

  emit roiChanged(roi);
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::commitPendingOrthogonalROI(ROISPtr roi)
{
  // añadir previa OROI al acumulador
  auto prevROI = m_ortogonalROITool->currentROI();
  if (prevROI)
  {
    auto mask = BinaryMaskSPtr<unsigned char>{new BinaryMask<unsigned char>{prevROI->bounds(), prevROI->spacing(), prevROI->origin()}};
    BinaryMask<unsigned char>::iterator it{mask.get()};
    it.goToBegin();
    while(!it.isAtEnd())
    {
      it.Set();
      ++it;
    }

    addMask(mask);
  }

  m_ortogonalROITool->setROI(roi); // this roi can now be edited
}

//-----------------------------------------------------------------------------
void ROIToolsGroup::addMask(const BinaryMaskSPtr<unsigned char> mask)
{
  if (m_accumulator)
  {
    expandAndDraw(m_accumulator.get(), mask);
  }
  else
  {
    setCurrentROI(ROISPtr{new ROI(mask)});
  }
}
