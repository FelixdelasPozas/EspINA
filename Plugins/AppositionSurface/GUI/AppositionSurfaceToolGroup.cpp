/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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

// plugin
#include "AppositionSurfaceToolGroup.h"
#include <Filter/AppositionSurfaceFilter.h>

// EspINA
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/AddCategoryCommand.h>
#include <Undo/AddRelationCommand.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QApplication>
#include <QUndoStack>
#include <QIcon>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>

using namespace EspINA;

const QString SAS = QObject::tr("SAS");

//-----------------------------------------------------------------------------
AppositionSurfaceToolGroup::AppositionSurfaceToolGroup(ModelAdapterSPtr model, QUndoStack *undoStack, ModelFactorySPtr factory, ViewManagerSPtr viewManager)
: ToolGroup(viewManager, QIcon(":/AppSurface.svg"), tr("Apposition Surface Tools"), nullptr)
, m_model    {model}
, m_factory  {factory}
, m_undoStack{undoStack}
, m_tool     {SASToolSPtr{new AppositionSurfaceTool{QIcon(":/AppSurface.svg"), tr("Create a synaptic apposition surface from selected segmentations.")}}}
, m_enabled  {true}
{
  m_tool->setToolTip("Create a synaptic apposition surface from selected segmentations.");
  connect(m_tool.get(), SIGNAL(triggered()), this, SLOT(createSAS()));

  connect(viewManager->selection().get(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

//-----------------------------------------------------------------------------
AppositionSurfaceToolGroup::~AppositionSurfaceToolGroup()
{
  disconnect(m_tool.get(), SIGNAL(triggered()), this, SLOT(createSAS()));
  disconnect(m_viewManager->selection().get(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::setEnabled(bool enable)
{
  m_enabled = enable;
  selectionChanged();
}

//-----------------------------------------------------------------------------
bool AppositionSurfaceToolGroup::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
ToolSList AppositionSurfaceToolGroup::tools()
{
  ToolSList tools;

  tools << m_tool;

  return tools;
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::selectionChanged()
{
  QString toolTip("Create a synaptic apposition surface from selected segmentations.");
  bool enabled = false;

  for(auto segmentation: m_viewManager->selection()->segmentations())
  {
    if (isSynapse(segmentation))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
    toolTip += QString("\n(Requires a selection of one or more segmentations from 'Synapse' taxonomy)");

  m_tool->setToolTip(toolTip);
  m_tool->setEnabled(enabled && m_enabled);
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::createSAS()
{
  auto segmentations = m_viewManager->selection()->segmentations();
  SegmentationAdapterList validSegmentations;
  for(auto seg: segmentations)
  {
    if (isSynapse(seg))
    {
      bool valid = true;
      for (auto item : m_model->relatedItems(seg, RELATION_OUT))
        if (item->type() == ItemAdapter::Type::SEGMENTATION)
        {
          SegmentationAdapterSPtr sasCandidate = std::dynamic_pointer_cast<SegmentationAdapter>(item);
          if (sasCandidate->category()->classificationName().startsWith("SAS/") ||
              sasCandidate->category()->classificationName().compare("SAS") == 0)
          {
            valid = false;
          }
        }

      if (valid)
        validSegmentations << seg;
    }
  }

  if(validSegmentations.empty())
  {
    QMessageBox::information(nullptr, tr("EspINA"), tr("Selected Synapses already have an associated Apposittion Surface."));
    return;
  }

  for(auto seg: validSegmentations)
  {
    InputSList inputs;
    inputs << seg->asInput();

    auto adapter = m_factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

    struct Data data(adapter, m_model->smartPointer(seg));
    m_executingTasks.insert(adapter.get(), data);

    connect(adapter.get(), SIGNAL(finished()), this, SLOT(finishedTask()));
    adapter->submit();
  }
}

//-----------------------------------------------------------------------------
void AppositionSurfaceToolGroup::finishedTask()
{
  auto filter = qobject_cast<FilterAdapterPtr>(sender());

  disconnect(filter, SIGNAL(finished()), this, SLOT(finishedTask()));

  if(!filter->isAborted())
    m_finishedTasks.insert(filter, m_executingTasks[filter]);

  m_executingTasks.remove(filter);

  if (!m_executingTasks.empty())
    return;

  // maybe all tasks have been aborted
  if(m_finishedTasks.empty())
    return;

  m_undoStack->beginMacro("Create Synaptic Apposition Surfaces");

  auto classification = m_model->classification();
  if (classification->category(SAS) == nullptr)
  {
    m_undoStack->push(new AddCategoryCommand(m_model->classification()->root(), SAS, m_model, QColor(255,255,0)));

    m_model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
    m_model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
    m_model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
  }

  CategoryAdapterSPtr category = classification->category(SAS);
  Q_ASSERT(category);

  SegmentationAdapterList createdSegmentations;

  for(auto filter: m_finishedTasks.keys())
  {
    auto segmentation = m_factory->createSegmentation(m_finishedTasks.value(filter).adapter, 0);
    segmentation->setCategory(category);

    auto extension = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
    std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension)->setOriginSegmentation(m_finishedTasks[filter].segmentation);
    segmentation->addExtension(extension);

    auto samples = QueryAdapter::samples(m_finishedTasks.value(filter).segmentation);
    Q_ASSERT(!samples.empty());

    m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
    m_undoStack->push(new AddRelationCommand(m_finishedTasks[filter].segmentation, segmentation, SAS, m_model));

    createdSegmentations << segmentation.get();
  }
  m_undoStack->endMacro();

  m_viewManager->updateSegmentationRepresentations(createdSegmentations);
  m_viewManager->updateViews();

  m_finishedTasks.clear();
}


//-----------------------------------------------------------------------------
bool AppositionSurfaceToolGroup::isSynapse(SegmentationAdapterPtr segmentation)
{
  return segmentation->category()->classificationName().contains(tr("Synapse"));
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::AppositionSurfaceTool(const QIcon& icon, const QString& text)
: m_action {new QAction{icon, text, nullptr}}
{
  connect(m_action, SIGNAL(triggered()), this, SLOT(activated()));
}

//-----------------------------------------------------------------------------
AppositionSurfaceTool::~AppositionSurfaceTool()
{
  disconnect(m_action, SIGNAL(triggered()), this, SLOT(activated()));
}

//-----------------------------------------------------------------------------
QList<QAction *> AppositionSurfaceTool::actions() const
{
  QList<QAction *> actions;
  actions << m_action;

  return actions;
}

