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
#include "AppositionSurfacePlugin.h"
#include "Analysis/SynapticAppositionSurfaceAnalysis.h"
#include "GUI/Settings/AppositionSurfaceSettings.h"
#include "GUI/FilterInspector/AppositionSurfaceFilterInspector.h"
#include "GUI/AppositionSurfaceAction.h"
#include "Undo/AppositionSurfaceCommand.h"

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaSettings.h>
#include <Undo/TaxonomiesCommand.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

// Qt
#include <QColorDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QString>
#include <QVariant>

using namespace EspINA;

//-----------------------------------------------------------------------------
AppositionSurface::AppositionSurface()
: m_factory(NULL)
, m_model(NULL)
, m_undoStack(NULL)
, m_viewManager(NULL)
, m_action(NULL)
, m_settings(ISettingsPanelPrototype(new AppositionSurfaceSettings()))
, m_extension(new AppositionSurfaceExtension())
{
  setObjectName("SinapticAppositionSurfacePlugin");
  setWindowTitle(tr("Sinaptic Apposition Surface Tool Bar"));
}

//-----------------------------------------------------------------------------
AppositionSurface::~AppositionSurface()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Apposition Surface Plugin";
//   qDebug() << "********************************************************";
  m_factory->unregisterSettingsPanel(m_settings.get());
  m_factory->unregisterSegmentationExtension(m_extension.get());
  // filters can't be unregistered, is this a problem?

  delete m_action;
}

//-----------------------------------------------------------------------------
void AppositionSurface::initToolBar(EspinaModel *model, QUndoStack *undoStack, ViewManager *viewManager)
{
  m_model = model;
  m_undoStack = undoStack;
  m_viewManager = viewManager;

  m_action = new AppositionSurfaceAction(m_viewManager, m_undoStack, m_model, this);
  m_action->setToolTip("Create a synaptic apposition surface from selected segmentations.");
  addAction(m_action);

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)), this,
          SLOT(selectionChanged(ViewManager::Selection, bool)));

  // for automatic computation of SAS
  connect(m_model, SIGNAL(segmentationAdded(SegmentationSPtr)),
          this, SLOT(segmentationAdded(SegmentationSPtr)));
}

//-----------------------------------------------------------------------------
void AppositionSurface::initFactoryExtension(EspinaFactory *factory)
{
  m_factory = factory;

  // register settings panel
  m_factory->registerSettingsPanel(m_settings.get());

  // register extension
  m_factory->registerSegmentationExtension(m_extension.get());

  // register filter
  factory->registerFilter(this, AppositionSurfaceCommand::FILTER_TYPE);
}

//-----------------------------------------------------------------------------
FilterSPtr AppositionSurface::createFilter(const QString &filter, const Filter::NamedInputs &inputs,
                                           const ModelItem::Arguments &args)
{
  Q_ASSERT(AppositionSurfaceCommand::FILTER_TYPE == filter);

  FilterSPtr asFilter = FilterSPtr(new AppositionSurfaceFilter(inputs, args, AppositionSurfaceCommand::FILTER_TYPE));
  SetBasicGraphicalRepresentationFactory(asFilter);

  Filter::FilterInspectorPtr asInspector(new AppositionSurfaceFilterInspector(asFilter));
  asFilter->setFilterInspector(asInspector);

  return asFilter;
}

//-----------------------------------------------------------------------------
QList<MenuEntry> AppositionSurface::menuEntries()
{
  QList<MenuEntry> entries;

  QStringList hierarchy;
  hierarchy << "Analysis" << tr("Apposition Surface");

  QAction *action = new QAction(tr("Synaptic Apposition Surface"), this);
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(createSynapticAppositionSurfaceAnalysis()));

  entries << MenuEntry(hierarchy, action);

  return entries;
}

typedef SynapticAppositionSurfaceAnalysis SASA;

//-----------------------------------------------------------------------------
void AppositionSurface::createSynapticAppositionSurfaceAnalysis()
{
  SegmentationList synapsis;

  SegmentationList selection = m_viewManager->selectedSegmentations();
  if (selection.isEmpty())
  {
    foreach(SegmentationSPtr segmentation, m_model->segmentations())
    {
      if (isSynapse(segmentation.get()))
      {
        synapsis << segmentation.get();
      }
    }
  } else
  {
    foreach(SegmentationPtr segmentation, selection)
    {
      if (isSynapse(segmentation))
      {
        synapsis << segmentation;
      }
    }
  }

  if (!synapsis.isEmpty())
  {
    TaxonomySPtr taxonomy = m_model->taxonomy();
    bool createTaxonomy = !taxonomy->element(SAS);
    if (createTaxonomy)
    {
      m_undoStack->beginMacro(tr("Apposition Surface"));
      m_undoStack->push(new AddTaxonomyElement(taxonomy->root().get(), SAS, m_model, QColor(255,255,0)));
      m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
      m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
      m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
    }

    SASA *analysis = new SASA(synapsis, m_model, m_undoStack, m_viewManager, this);

    if (createTaxonomy)
      m_undoStack->endMacro();

    analysis->show();
  }
  else
  {
    QMessageBox::warning(this, ESPINA, tr("Current analysis does not contain any synapses"));
  }
}

//-----------------------------------------------------------------------------
void AppositionSurface::selectionChanged(ViewManager::Selection selection, bool unused)
{
  QString toolTip("Create a synaptic apposition surface from selected segmentations.");
  bool enabled = false;

  foreach(PickableItemPtr item, selection)
  {
    if (item->type() == SEGMENTATION && isSynapse(segmentationPtr(item)))
    {
      enabled = true;
      break;
    }
  }

  if (!enabled)
    toolTip += QString("\n(Requires a selection of one or more segmentations from 'Synapse' taxonomy)");

  m_action->setToolTip(toolTip);
  m_action->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void AppositionSurface::reset()
{
}

//-----------------------------------------------------------------------------
bool AppositionSurface::isSynapse(SegmentationPtr segmentation)
{
  return segmentation->taxonomy()->qualifiedName().contains(tr("Synapse"));
}


//-----------------------------------------------------------------------------
void AppositionSurface::segmentationAdded(SegmentationSPtr segmentation)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.beginGroup("Apposition Surface");
  if (!settings.contains("Automatic Computation For Synapses") || !settings.value("Automatic Computation For Synapses").toBool())
    return;

  // filter->executed() marks the segmentation as created in session time, not loaded from a file.
  if (isSynapse(segmentation.get()) && segmentation->filter()->executed())
  {
    // must check if the segmentation already has a SAS, as this call
    // could be the result of a redo() in a UndoCommand
    foreach(ModelItemSPtr item, segmentation->relatedItems(EspINA::OUT))
      if (item->type() == SEGMENTATION)
      {
        SegmentationSPtr segmentation = segmentationPtr(item);
        if (segmentation->taxonomy()->qualifiedName().startsWith("SAS/") || segmentation->taxonomy()->qualifiedName().compare("SAS") == 0)
          return;
      }

    SegmentationList synapses;
    synapses << segmentation.get();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    {
      SegmentationSList createdSegmentations;
      m_undoStack->beginMacro(tr("Apposition Surface"));
      {
        TaxonomySPtr taxonomy = m_model->taxonomy();
        if (!taxonomy->element(SAS))
        {
          m_undoStack->push(new AddTaxonomyElement(taxonomy->root().get(), SAS, m_model, QColor(255,255,0)));
          m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
          m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
          m_model->taxonomy()->element(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
        }
        m_undoStack->push(new AppositionSurfaceCommand(synapses, m_model, m_viewManager, createdSegmentations));
      }
      m_model->emitSegmentationAdded(createdSegmentations);
      m_undoStack->endMacro();
    }
    QApplication::restoreOverrideCursor();
  }
}

Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, EspINA::AppositionSurface)

