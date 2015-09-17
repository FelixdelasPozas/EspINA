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
#include "RestrictToolGroup.h"
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include "DeleteROITool.h"
#include "FreehandROITool.h"
#include "OrthogonalROITool.h"
#include <Undo/ROIUndoCommand.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::ROI;

class RestrictToolGroup::DefineOrthogonalROICommand
: public QUndoCommand
{
public:
  explicit DefineOrthogonalROICommand(ROISPtr roi, RestrictToolGroup *tool);

  virtual void redo() override;

  virtual void undo() override;

private:
  ROISPtr            m_ROI;
  RestrictToolGroup *m_tool;
  ROISPtr            m_prevROI;
};

//-----------------------------------------------------------------------------
RestrictToolGroup::DefineOrthogonalROICommand::DefineOrthogonalROICommand(ROISPtr roi, RestrictToolGroup* tool)
: m_ROI(roi)
, m_tool(tool)
, m_prevROI{tool->m_orthogonalROI->currentROI()}
{
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::DefineOrthogonalROICommand::redo()
{
  m_tool->commitPendingOrthogonalROI(m_ROI);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::DefineOrthogonalROICommand::undo()
{
  m_tool->m_orthogonalROI->setROI(m_prevROI);
}

//-----------------------------------------------------------------------------
class RestrictToolGroup::ConsumeROICommand
: public QUndoCommand
{
  public:
    explicit ConsumeROICommand(RestrictToolGroup *tool);

    virtual void redo() override;

    virtual void undo() override;
  private:
    RestrictToolGroup *m_tool;
    ROISPtr m_roi;
};

//-----------------------------------------------------------------------------
RestrictToolGroup::ConsumeROICommand::ConsumeROICommand(RestrictToolGroup *tool)
: m_tool{tool}
, m_roi {tool->currentROI()}
{
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::ConsumeROICommand::redo()
{
  m_tool->setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::ConsumeROICommand::undo()
{
  m_tool->setCurrentROI(m_roi);
}

//-----------------------------------------------------------------------------
class RestrictToolGroup::DefineManualROICommand
: public QUndoCommand
{
public:
  explicit DefineManualROICommand(const BinaryMaskSPtr<unsigned char> mask, RestrictToolGroup *tool);

  virtual void redo() override;

  virtual void undo() override;

private:
  const BinaryMaskSPtr<unsigned char> m_mask;
  RestrictToolGroup *m_tool;
  ROISPtr            m_oROI;
  bool               m_firstROI;

  itkVolumeType::Pointer m_image;
  Bounds                 m_ROIbounds;
};

//-----------------------------------------------------------------------------
RestrictToolGroup::DefineManualROICommand::DefineManualROICommand(const BinaryMaskSPtr<unsigned char> mask, RestrictToolGroup* tool)
: m_mask     {mask}
, m_tool     {tool}
, m_oROI     {tool->m_orthogonalROI->currentROI()}
, m_firstROI {tool->m_accumulator == nullptr}
, m_image    {nullptr}
, m_ROIbounds{Bounds()}
{
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::DefineManualROICommand::redo()
{
  m_tool->commitPendingOrthogonalROI(nullptr);

  if(!m_firstROI)
  {
    auto currentROI = m_tool->currentROI();
    m_ROIbounds = currentROI->bounds();

    if(intersect(currentROI->bounds(), m_mask->bounds().bounds(), m_mask->spacing()))
    {
      auto bounds = intersection(currentROI->bounds(), m_mask->bounds().bounds());
      m_image  = currentROI->itkImage(bounds);
    }
  }

  m_tool->addManualROI(m_mask);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::DefineManualROICommand::undo()
{
  if (m_firstROI)
  {
    m_tool->setCurrentROI(m_oROI);
  }
  else
  {
    auto currentROI = m_tool->currentROI();

    if(currentROI)
    {
      currentROI->resize(m_ROIbounds);
    }

    if(m_image)
    {
      if(currentROI)
      {
        currentROI->draw(m_image);
      }
      else
      {
        auto mask = std::make_shared<BinaryMask<unsigned char>>(m_image);

        m_tool->setCurrentROI(std::make_shared<ROI>(mask));
      }
    }

    if (m_oROI)
    {
      m_tool->m_orthogonalROI->setROI(m_oROI);
    }
  }
}

//-----------------------------------------------------------------------------
RestrictToolGroup::RestrictToolGroup(ROISettings*     settings,
                                     Support::Context &context)
: ToolGroup      {":/espina/toolgroup_restrict.svg", tr("ROI")}
, m_context      (context)
, m_freehandROI  {new FreehandROITool(context, this)}
, m_orthogonalROI{new OrthogonalROITool(settings, context, this)}
, m_deleteROI    {new DeleteROITool(context, this)}
, m_enabled      {true}
, m_visible      {true}
, m_color        {Qt::yellow}
{
  setColor(m_color);

  setToolTip(tr("Region of Interest"));

  m_orthogonalROI->setOrder("0-0");
  m_freehandROI->setOrder("0-1");
  m_deleteROI->setOrder("1");

  addTool(m_orthogonalROI);
  addTool(m_freehandROI);
  addTool(m_deleteROI);

  connect(m_freehandROI.get(), SIGNAL(roiDefined(BinaryMaskSPtr<unsigned char>)),
          this,                SLOT(onManualROIDefined(BinaryMaskSPtr<unsigned char>)));

  connect(m_orthogonalROI.get(), SIGNAL(roiDefined(ROISPtr)),
          this,                  SLOT(onOrthogonalROIDefined(ROISPtr)));

  m_deleteROI->setEnabled(false);
}

//-----------------------------------------------------------------------------
RestrictToolGroup::~RestrictToolGroup()
{
  setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::setCurrentROI(ROISPtr roi)
{
  if(m_accumulator)
  {
    m_context.viewState().removeTemporalRepresentations(m_roiPrototypes);

    m_accumulator  .reset();
    m_roiPrototypes.reset();
  }

  if (roi && roi->isOrthogonal())
  {
    m_orthogonalROI->setROI(roi);
  }
  else
  {
    m_orthogonalROI->setROI(nullptr);
  }

  if (roi && !roi->isOrthogonal())
  {
    m_accumulator = roi;

    auto roi2D = std::make_shared<ROIWidget>(m_accumulator);
    roi2D->setColor(m_color);

    m_roiPrototypes = std::make_shared<TemporalPrototypes>(roi2D, TemporalRepresentation3DSPtr());

    m_context.viewState().addTemporalRepresentations(m_roiPrototypes);
  }

  emit roiChanged(roi);
}

//-----------------------------------------------------------------------------
ROISPtr RestrictToolGroup::currentROI()
{
  auto roi = m_orthogonalROI->currentROI();

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
void RestrictToolGroup::consumeROI()
{
  undoStackPush(new ConsumeROICommand{this});
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::setColor(const QColor& color)
{
  m_color = color;

  m_freehandROI->setColor(color);
  m_orthogonalROI->setColor(color);
}

//-----------------------------------------------------------------------------
bool RestrictToolGroup::hasValidROI() const
{
  return m_accumulator || m_orthogonalROI->currentROI();
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::setVisible(bool visible)
{
  if (m_visible == visible) return;

  if (m_accumulator)
  {
    if (visible)
    {
      m_context.viewState().addTemporalRepresentations(m_roiPrototypes);
    }
    else
    {
      m_context.viewState().removeTemporalRepresentations(m_roiPrototypes);
    }
  }

  if (m_orthogonalROI->currentROI())
  {
    m_orthogonalROI->setVisible(visible);
  }

  m_visible = visible;
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::onManualROIDefined(BinaryMaskSPtr<unsigned char> roi)
{
  commitPendingOrthogonalROI(nullptr);

  undoStackPush(new DefineManualROICommand{roi, this});

  m_context.roiProvider()->setProvider(this);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::onOrthogonalROIDefined(ROISPtr roi)
{
  undoStackPush(new DefineOrthogonalROICommand{roi, this});

  m_context.roiProvider()->setProvider(this);

  emit roiChanged(roi);
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::undoStackPush(QUndoCommand *command)
{
  auto undoStack = m_context.undoStack();

  if(hasValidROI())
  {
    undoStack->beginMacro(tr("Modify ROI"));
  }
  else
  {
    undoStack->beginMacro(tr("Create ROI"));
  }
  undoStack->push(command);
  undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::commitPendingOrthogonalROI(ROISPtr roi)
{
  // añadir previa OROI al acumulador
  auto prevROI = m_orthogonalROI->currentROI();

  if (prevROI)
  {
    addOrthogonalROI(prevROI->bounds());
  }

  m_orthogonalROI->setROI(roi); // this roi can now be edited
}

//-----------------------------------------------------------------------------
void RestrictToolGroup::addOrthogonalROI(const VolumeBounds& bounds)
{
  if (m_accumulator)
  {
    m_accumulator->resize(boundingBox(m_accumulator->bounds(), bounds));
    m_accumulator->draw(bounds, SEG_VOXEL_VALUE);
  }
  else
  {
    auto roi = std::make_shared<ROI>(bounds);

    roi->draw(bounds, SEG_VOXEL_VALUE);

    setCurrentROI(roi);
  }

}

//-----------------------------------------------------------------------------
void RestrictToolGroup::addManualROI(const BinaryMaskSPtr<unsigned char> mask)
{
  if (m_accumulator)
  {
    expandAndDraw(m_accumulator.get(), mask);

    if(m_accumulator->isEmpty())
    {
      setCurrentROI(nullptr);
    }
  }
  else
  {
    setCurrentROI(std::make_shared<ROI>(mask));
  }
}
