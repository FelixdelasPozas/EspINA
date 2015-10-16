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
#include "DefaultView.h"
#include <App/EspinaMainWindow.h>
#include <Core/Utils/ListUtils.hxx>
#include <Dialogs/Dialog3D/Dialog3D.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Support/Context.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <ToolGroups/Visualize/VisualizeToolGroup.h>

// Qt
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QGroupBox>
#include <QMainWindow>
#include <QSettings>
#include <QVBoxLayout>
#include <QMenu>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::Support::Representations::Utils;

const QString DEFAULT_VIEW_SETTINGS = "DefaultView";
const QString SHOW_RULER_KEY        = "ShowRuler";
const QString SHOW_THUMBNAIL_KEY    = "ShowThumbnail";

const QString DefaultView::FIT_TO_SLICES_KEY = "FitToSlices";

//----------------------------------------------------------------------------
DefaultView::DefaultView(Support::Context &context,
                         QMainWindow      *parent)
: WithContext(context)
, m_sources  (getModel(), getViewState())
, m_viewXY   {new View2D(context.viewState(), Plane::XY)}
, m_viewYZ   {new View2D(context.viewState(), Plane::YZ)}
, m_viewXZ   {new View2D(context.viewState(), Plane::XZ)}
{
  setObjectName("viewXY");

  setLayout(new QVBoxLayout());
  layout()->addWidget(m_viewXY);
  layout()->setMargin(0);

  m_panelYZ = new Panel(tr("ZY"), context, parent);
  m_panelYZ->setObjectName("DockZY");
  m_panelYZ->setWidget(m_viewYZ);

  m_panelXZ = new Panel(tr("XZ"), context, parent);
  m_panelXZ->setObjectName("xzDock");
  m_panelXZ->setWidget(m_viewXZ);

  m_dialog3D = new Dialog3D(context);

  parent->addDockWidget(Qt::RightDockWidgetArea, m_panelYZ);
  parent->addDockWidget(Qt::RightDockWidgetArea, m_panelXZ);

  initView(m_viewXY, parent);
  initView(m_viewXZ, parent);
  initView(m_viewYZ, parent);
  initDialog3D(m_dialog3D, parent);

  parent->setCentralWidget(this);

  //auto colorEngine  = std::dynamic_pointer_cast<MultiColorEngine>(m_context.colorEngine());
  connect(getContext().colorEngine().get(), SIGNAL(modified()),
          this,                             SLOT(onColorEngineModified()));
}

//-----------------------------------------------------------------------------
DefaultView::~DefaultView()
{
  if(m_dialog3D->isVisible())
  {
    m_dialog3D->hide();
  }

  delete m_dialog3D;
}

//-----------------------------------------------------------------------------
void DefaultView::addRepresentation(const Representation& representation)
{
  for(auto repSwitch: representation.Switches)
  {
    if(repSwitch->supportedViews().testFlag(ViewType::VIEW_3D))
    {
      m_dialog3D->addRepresentationSwitch(repSwitch);
    }
  }

  for (auto manager : representation.Managers)
  {
    addRepresentationManager(manager);
  }

  for (auto pool : representation.Pools)
  {
    pool->setPipelineSources(&m_sources);
    m_pools << pool;
  }
}

//-----------------------------------------------------------------------------
void DefaultView::createViewMenu(QMenu* menu)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup(DEFAULT_VIEW_SETTINGS);

  auto fs = settings.value(FIT_TO_SLICES_KEY,  true).toBool();

  settings.endGroup();

  auto fitToSlices = new QAction(tr("Fit To Slices"), menu);
  fitToSlices->setCheckable(true);
  fitToSlices->setChecked(fs);
  menu->addAction(fitToSlices);
  connect(fitToSlices, SIGNAL(toggled(bool)),
          this,        SLOT(setFitToSlices(bool)));

  setFitToSlices(fs);
}

//-----------------------------------------------------------------------------
Panel* DefaultView::panelXZ()
{
  return m_panelXZ;
}

//-----------------------------------------------------------------------------
Panel* DefaultView::panelYZ()
{
  return m_panelYZ;
}

//-----------------------------------------------------------------------------
Dialog3D* DefaultView::dialog3D()
{
  return m_dialog3D;
}

//-----------------------------------------------------------------------------
void DefaultView::setRulerVisibility(bool visible)
{
  m_viewXY->setScaleVisibility(visible);
  m_viewYZ->setScaleVisibility(visible);
  m_viewXZ->setScaleVisibility(visible);
}

//-----------------------------------------------------------------------------
void DefaultView::showThumbnail(bool visible)
{
  m_viewXY->setThumbnailVisibility(visible);
  m_viewYZ->setThumbnailVisibility(visible);
  m_viewXZ->setThumbnailVisibility(visible);
}

//-----------------------------------------------------------------------------
void DefaultView::setFitToSlices(bool enabled)
{
  if(enabled != getViewState().fitToSlices())
  {
    auto resolution = NmVector3{1,1,1};

    if(enabled)
    {
      auto channels = getModel()->channels();
      resolution = channels.first()->output()->spacing();

      for(auto channel: channels)
      {
        for(auto i: {0,1,2})
        {
          auto channelResolution = channel->output()->spacing();
          resolution[i] = std::min(resolution[i], channelResolution[i]);
        }
      }
    }

    getViewState().setFitToSlices(enabled);
    getViewState().coordinateSystem()->setResolution(resolution);

    auto channelList      = toRawList<ViewItemAdapter>(getModel()->channels());
    auto segmentationList = toRawList<ViewItemAdapter>(getModel()->segmentations());

    getViewState().invalidateRepresentations(channelList + segmentationList);

    ESPINA_SETTINGS(settings);
    settings.beginGroup(DEFAULT_VIEW_SETTINGS);
    settings.setValue(FIT_TO_SLICES_KEY, enabled);
    settings.endGroup();
  }
}

//-----------------------------------------------------------------------------
void DefaultView::onColorEngineModified()
{
  auto  segmentations   = getModel()->segmentations();
  auto  invalidateItems = toRawList<ViewItemAdapter>(segmentations);

  getViewState().invalidateRepresentationColors(invalidateItems);
}

//-----------------------------------------------------------------------------
void DefaultView::initView(RenderView* view, QMainWindow *parent)
{
  m_views << view;

  connect(parent, SIGNAL(analysisClosed()),
          view,   SLOT(reset()));
}

//-----------------------------------------------------------------------------
void DefaultView::initDialog3D(Dialog3D *dialog, QMainWindow *parent)
{
  auto view = dialog->renderView();

  initView(view, parent);
}

//-----------------------------------------------------------------------------
QList<RenderView*> ESPINA::DefaultView::renderviews() const
{
  return m_views;
}

//-----------------------------------------------------------------------------
void DefaultView::addRepresentationManager(RepresentationManagerSPtr manager)
{
  for (auto view : m_views)
  {
    if (manager->supportedViews().testFlag(view->type()))
    {
      view->addRepresentationManager(manager->clone());
    }
  }
}
