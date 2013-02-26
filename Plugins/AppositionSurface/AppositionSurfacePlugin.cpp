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
#include "GUI/Settings/AppositionSurfaceSettings.h"
#include "GUI/FilterInspector/AppositionSurfaceFilterInspector.h"
#include "GUI/AppositionSurfaceAction.h"
#include "Undo/AppositionSurfaceCommand.h"

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaSettings.h>

// Qt
#include <QColorDialog>
#include <QSettings>
#include <QDebug>

namespace EspINA
{

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
    qDebug() << "********************************************************";
    qDebug() << "              Destroying Apposition Surface Plugin";
    qDebug() << "********************************************************";
    m_factory->unregisterSettingsPanel(m_settings.data());
    m_factory->unregisterSegmentationExtension(m_extension.data());
    m_factory->unregisterRenderer(m_renderer.data());
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

    // register renderer
    m_renderer = AppositionSurfaceRendererSPtr(new AppositionSurfaceRenderer(m_viewManager));
    m_factory->registerRenderer(m_renderer.data());

    // for automatic computation of SAS
    connect(m_model, SIGNAL(segmentationAdded(SegmentationSPtr)),
            this, SLOT(segmentationAdded(SegmentationSPtr)));
  }

  //-----------------------------------------------------------------------------
  void AppositionSurface::initFactoryExtension(EspinaFactoryPtr factory)
  {
    m_factory = factory;

    // register settings panel
    m_factory->registerSettingsPanel(m_settings.data());

    // register extension
    m_factory->registerSegmentationExtension(m_extension.data());

    // register filter
    factory->registerFilter(this, AppositionSurfaceCommand::FILTER_TYPE);
  }

  //-----------------------------------------------------------------------------
  FilterSPtr AppositionSurface::createFilter(const QString &filter, const Filter::NamedInputs &inputs,
      const ModelItem::Arguments &args)
  {
    Q_ASSERT(AppositionSurfaceCommand::FILTER_TYPE == filter);

    FilterSPtr asFilter = FilterSPtr(new AppositionSurfaceFilter(inputs, args, AppositionSurfaceCommand::FILTER_TYPE));

    Filter::FilterInspectorPtr asInspector(new AppositionSurfaceFilterInspector(asFilter));
    asFilter->setFilterInspector(asInspector);

    return asFilter;
  }

  //-----------------------------------------------------------------------------
  void AppositionSurface::selectionChanged(ViewManager::Selection selection, bool unused)
  {
    QString toolTip("Create a synaptic apposition surface from selected segmentations.");
    bool enabled = false;

    foreach(PickableItemPtr item, selection)
      if ((item->type() == SEGMENTATION) && (SegmentationPtr(item)->taxonomy()->qualifiedName().contains(QString("Synapse"))))
      {
        enabled = true;
        break;
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
  void AppositionSurface::segmentationAdded(SegmentationSPtr seg)
  {
    // TODO: right now there is no way to know if a segmentation has been loaded
    // from file or created in a session as this method get called every time a
    // seg is added to the model, no matter what.
  }

}

Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, EspINA::AppositionSurface)


