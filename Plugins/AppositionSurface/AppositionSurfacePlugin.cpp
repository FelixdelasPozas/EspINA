/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include <Core/Analysis/Segmentation.h>
#include <Core/Extensions/ExtensionFactory.h>
#include <Extensions/ExtensionUtils.h>
#include <Filter/AppositionSurfaceFilter.h>
#include <Filter/SASDataFactory.h>
#include <GUI/Analysis/SASAnalysisDialog.h>
#include <GUI/AppositionSurfaceTool.h>
#include <GUI/Settings/AppositionSurfaceSettings.h>

// ESPINA
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <Undo/AddCategoryCommand.h>
#include <Support/Utils/SelectionUtils.h>
#include <Support/Settings/EspinaSettings.h>
#include <Undo/AddSegmentations.h>
#include <Undo/AddRelationCommand.h>

// Qt
#include <QColorDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QVariant>

const QString SAS = QObject::tr("SAS");
const QString SAS_PREFIX = QObject::tr("SAS ");

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Settings;

//-----------------------------------------------------------------------------
FilterTypeList AppositionSurfacePlugin::ASFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << AS_FILTER;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr AppositionSurfacePlugin::ASFilterFactory::createFilter(InputSList    inputs,
                                                            const Filter::Type& type,
                                                            SchedulerSPtr       scheduler) const
throw (Unknown_Filter_Exception)
{

  if (type != AS_FILTER) throw Unknown_Filter_Exception();

  auto filter = std::make_shared<AppositionSurfaceFilter>(inputs, type, scheduler);

  filter->setDataFactory(std::make_shared<SASDataFactory>());

  return filter;
}

//-----------------------------------------------------------------------------
AppositionSurfacePlugin::AppositionSurfacePlugin()
: m_context         {nullptr}
, m_settings        {new AppositionSurfaceSettings()}
, m_extensionFactory{new ASExtensionFactory()}
, m_filterFactory   {new ASFilterFactory()}
, m_delayedAnalysis {false}
{
  QStringList hierarchy;
  hierarchy << "Reports";

  QAction *action = new QAction(tr("Synaptic Apposition Surface"), this);
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(createSASAnalysis()));

  m_menuEntry = MenuEntry(hierarchy, action);
}

//-----------------------------------------------------------------------------
AppositionSurfacePlugin::~AppositionSurfacePlugin()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Apposition Surface Plugin";
//   qDebug() << "********************************************************";
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::init(Context &context)
{
  if (m_context)
  {
    qWarning() << "Already Initialized AppositionSurfacePlugin";
    Q_ASSERT(false);
  }

  m_context = &context;

  // for automatic computation of SAS
  connect(m_context->model().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
          this,                     SLOT(segmentationsAdded(ViewItemAdapterSList)));
}

//-----------------------------------------------------------------------------
SegmentationExtensionFactorySList AppositionSurfacePlugin::segmentationExtensionFactories() const
{
  SegmentationExtensionFactorySList extensionFactories;

  extensionFactories << m_extensionFactory;

  return extensionFactories;
}

//-----------------------------------------------------------------------------
FilterFactorySList AppositionSurfacePlugin::filterFactories() const
{
  FilterFactorySList factories;

  factories << m_filterFactory;

  return factories;
}

//-----------------------------------------------------------------------------
QList<CategorizedTool> AppositionSurfacePlugin::tools() const
{
  QList<CategorizedTool> tools;

  auto plugin = const_cast<AppositionSurfacePlugin *>(this);

  tools << CategorizedTool(ToolCategory::SEGMENT, std::make_shared<AppositionSurfaceTool>(plugin, *m_context));

  return tools;
}

//-----------------------------------------------------------------------------
SettingsPanelSList AppositionSurfacePlugin::settingsPanels() const
{
  SettingsPanelSList settingsPanels;

  settingsPanels << m_settings;

  return settingsPanels;
}

//-----------------------------------------------------------------------------
QList<MenuEntry> AppositionSurfacePlugin::menuEntries() const
{
  QList<MenuEntry> entries;

  entries << m_menuEntry;

  return entries;
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::createSASAnalysis()
{
  Q_ASSERT(m_context);

  SegmentationAdapterList synapsis;

   for(auto segmentation: defaultReportInputSegmentations(getSelection(*m_context), m_context->model()))
   {
     if (isValidSynapse(segmentation))
     {
       synapsis << segmentation;
     }
   }

   if (synapsis.isEmpty())
   {
     QMessageBox::warning(nullptr, tr("ESPINA"), tr("Current analysis does not contain any synapses"));
     return;
   }

   auto undoStack = m_context->undoStack();
   auto model     = m_context->model();
   auto factory   = m_context->factory();

   if (model->classification()->category(SAS) == nullptr)
   {
     undoStack->beginMacro(tr("Apposition Surface"));
     undoStack->push(new AddCategoryCommand(model->classification()->root(), SAS, model, QColor(255, 255, 0)));
     undoStack->endMacro();

     model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
     model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
     model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
   }

   // check segmentations for SAS and create it if needed
   for (auto segmentation : synapsis)
   {
     auto sasItems = model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS);
     if (sasItems.empty())
     {
       if (!m_delayedAnalysis)
       {
         m_delayedAnalysis = true;
         m_analysisSynapses = synapsis;
         QApplication::setOverrideCursor(Qt::WaitCursor);
       }

       InputSList inputs;
       inputs << segmentation->asInput();

       auto filter = factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

       struct Data data(filter, model->smartPointer(segmentation));
       m_executingTasks.insert(filter.get(), data);

       connect(filter.get(), SIGNAL(finished()),
               this,         SLOT(finishedTask()));

       Task::submit(filter);
     }
     else
     {
       Q_ASSERT(sasItems.size() == 1);
       auto sas        = std::dynamic_pointer_cast<SegmentationAdapter>(sasItems.first());
       auto extensions = sas->extensions();
       auto extension  = factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
       std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension)->setOriginSegmentation(model->smartPointer(segmentation));
       extensions->add(extension);
     }
  }

  if (!m_delayedAnalysis)
  {
    SASAnalysisDialog *analysis = new SASAnalysisDialog(synapsis, *m_context);
    analysis->exec();

    delete analysis;
  };
}

//-----------------------------------------------------------------------------
bool AppositionSurfacePlugin::isValidSynapse(SegmentationAdapterPtr segmentation)
{
  bool isValidCategory = segmentation->category()->classificationName().contains(tr("Synapse"));
  bool hasRequiredData = hasVolumetricData(segmentation->output());

  return (isValidCategory && hasRequiredData);
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::segmentationsAdded(ViewItemAdapterSList segmentations)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup("Apposition Surface");
  if (!settings.contains("Automatic Computation For Synapses") || !settings.value("Automatic Computation For Synapses").toBool())
    return;

  auto model   = m_context->model();
  auto factory = m_context->factory();

  SegmentationAdapterList validSegmentations;
  for(auto item : segmentations)
  {
    auto segmentation = segmentationPtr(item.get());

    if(isValidSynapse(segmentation) && segmentation->filter()->hasFinished())
    {
      bool valid = true;
      // must check if the segmentation already has a SAS, as this call
      // could be the result of a redo() in a UndoCommand
      for(auto relatedItem : model->relatedItems(segmentation, RELATION_OUT))
      {
        if (isSegmentation(relatedItem.get()) && isSAS(relatedItem))
        {
          valid = false;
          break;
        }
      }

      if (valid)
      {
        validSegmentations << segmentation;
      }
    }
  }

  for(auto seg: validSegmentations)
  {
    InputSList inputs;
    inputs << seg->asInput();

    auto filter = factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

    struct Data data(filter, model->smartPointer(seg));
    m_executingTasks.insert(filter.get(), data);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(finishedTask()));

    Task::submit(filter);
  }
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::finishedTask()
{
  auto model     = m_context->model();
  auto factory   = m_context->factory();
  auto undoStack = m_context->undoStack();

  auto filter = dynamic_cast<FilterPtr>(sender());
  disconnect(filter, SIGNAL(finished()), this, SLOT(finishedTask()));

  if(!filter->isAborted())
  {
    m_finishedTasks.insert(filter, m_executingTasks[filter]);
  }

  m_executingTasks.remove(filter);

  if (!m_executingTasks.empty())
    return;

  // maybe all tasks have been aborted.
  if(m_finishedTasks.empty()) return;

  undoStack->beginMacro("Create Synaptic Apposition Surfaces");

  auto classification = model->classification();
  if (classification->category(SAS) == nullptr)
  {
    undoStack->push(new AddCategoryCommand(model->classification()->root(), SAS, model, QColor(255,255,0)));

    model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
    model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
    model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
  }

  CategoryAdapterSPtr category = classification->category(SAS);

  // we don't have an operator< for SampleAdapterSList so can't use a QMap and must use two lists to avoid
  // adding the segmentations one by one. Each segmentation can have a different sample list.
  QVector<SampleAdapterSList> usedSamples;
  QVector<SegmentationAdapterSList> createdSegmentations;
  int index = 0;

  RelationList createdRelations;
  ViewItemAdapterList segmentationsToUpdate;

  for(auto filter: m_finishedTasks.keys())
  {
    auto segmentation = factory->createSegmentation(m_finishedTasks.value(filter).adapter, 0);
    segmentation->setCategory(category);
    segmentation->setData(SAS_PREFIX + QString::number(m_finishedTasks[filter].segmentation->number()), Qt::EditRole);

    auto extensions   = segmentation->extensions();
    auto extension    = factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
    auto sasExtension = std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension);
    
    sasExtension->setOriginSegmentation(m_finishedTasks[filter].segmentation);
    extensions->add(sasExtension);

    auto samples = QueryAdapter::samples(m_finishedTasks.value(filter).segmentation);
    Q_ASSERT(!samples.empty());

    for(index = 0; index < usedSamples.size(); ++index)
      if(samples == usedSamples[index])
        break;

    if(index == usedSamples.size())
    {
      usedSamples.push_back(samples);
      SegmentationAdapterSList list;
      list << segmentation;
      createdSegmentations.push_back(list);
    }
    else
    {
      createdSegmentations[index] << segmentation;
    }

    Relation relation;
    relation.ancestor = m_finishedTasks[filter].segmentation;
    relation.successor = segmentation;
    relation.relation = SAS;

    createdRelations << relation;

    segmentationsToUpdate << segmentation.get();
  }

  // add segmentations by their related sample list
  for(index = 0; index < usedSamples.size(); ++index)
  {
    undoStack->push(new AddSegmentations(createdSegmentations[index], usedSamples[index], model));
  }

  // add relations
  undoStack->push(new AddRelationsCommand(createdRelations, model));
  undoStack->endMacro();

  m_context->representationInvalidator().invalidateRepresentations(segmentationsToUpdate);

  m_finishedTasks.clear();

  if(m_delayedAnalysis)
  {
    QApplication::restoreOverrideCursor();
    SASAnalysisDialog dialog(m_analysisSynapses, *m_context);
    dialog.exec();

    m_delayedAnalysis = false;
    m_analysisSynapses.clear();
  }
}

//-----------------------------------------------------------------------------
bool AppositionSurfacePlugin::isSAS(ItemAdapterSPtr item)
{
  bool result = false;

  auto sas = segmentationPtr(item.get());

  if (sas)
  {
    auto category = sas->category()->classificationName();

    result = category.startsWith("SAS/") || category.compare("SAS") == 0;
  }

  return result;
}

//-----------------------------------------------------------------------------
SegmentationAdapterPtr AppositionSurfacePlugin::segmentationSAS(SegmentationAdapterPtr segmentation)
{
  auto relatedItems = segmentation->model()->relatedItems(segmentation, RelationType::RELATION_OUT, SAS);

  Q_ASSERT(relatedItems.size() == 1);

  return segmentationPtr(relatedItems.first().get());
}


Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, ESPINA::AppositionSurfacePlugin)

