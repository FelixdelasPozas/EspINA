#include "CountingFramePlugin.h"

#include "Panel.h"
#include "ColorEngines/CountingFrameColorEngine.h"
#include "Extensions/CountingFrameFactories.h"
#include "Representations/RepresentationFactory.h"

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFramePlugin::CountingFramePlugin()
: m_undoStack                   {nullptr}
, m_colorEngine                 {NamedColorEngine()}
, m_dockWidget                  {nullptr}
{
}

//------------------------------------------------------------------------
CountingFramePlugin::~CountingFramePlugin()
{
}

//------------------------------------------------------------------------
void CountingFramePlugin::init(ModelAdapterSPtr model,
                               ViewManagerSPtr  viewManager,
                               ModelFactorySPtr factory,
                               SchedulerSPtr    scheduler,
                               QUndoStack*      undoStack)
{
  m_model       = model;
  m_viewManager = viewManager;
  m_scheduler   = scheduler;
  m_undoStack   = undoStack;

  m_colorEngine = NamedColorEngine("Counting Frame", std::make_shared<CountingFrameColorEngine>());
  m_representationFactory = std::make_shared<RepresentationFactory>(m_manager);

  m_dockWidget = new Panel(&m_manager, m_model, m_viewManager, m_scheduler);

  m_channelExtensionFactory      = std::make_shared<ChannelExtensionFactoryCF>(const_cast<CountingFrameManager *>(&m_manager), m_scheduler);
  m_segmentationExtensionFactory = std::make_shared<SegmentationExtensionFactoryCF>();
}

//------------------------------------------------------------------------
ChannelExtensionFactorySList CountingFramePlugin::channelExtensionFactories() const
{
  ChannelExtensionFactorySList factories;

  factories << m_channelExtensionFactory;

  return factories;
}

//------------------------------------------------------------------------
SegmentationExtensionFactorySList CountingFramePlugin::segmentationExtensionFactories() const
{
  SegmentationExtensionFactorySList factories;

  factories << m_segmentationExtensionFactory;

  return factories;
}

//------------------------------------------------------------------------
NamedColorEngineSList CountingFramePlugin::colorEngines() const
{
  NamedColorEngineSList engines;

  engines << m_colorEngine;

  return engines;
}

//------------------------------------------------------------------------
RepresentationFactorySList CountingFramePlugin::representationFactories() const
{
  RepresentationFactorySList factories;

  factories << m_representationFactory;

  return factories;
}

//------------------------------------------------------------------------
QList<ToolGroup* > CountingFramePlugin::toolGroups() const
{
  return QList<ToolGroup *>();
}

//------------------------------------------------------------------------
FilterFactorySList CountingFramePlugin::filterFactories() const
{
  return FilterFactorySList();
}

//------------------------------------------------------------------------
QList<DockWidget *> CountingFramePlugin::dockWidgets() const
{
  QList<DockWidget *> docks;

  docks << m_dockWidget;

  return docks;
}

//------------------------------------------------------------------------
SettingsPanelSList CountingFramePlugin::settingsPanels() const
{
  return SettingsPanelSList();
}

//------------------------------------------------------------------------
QList<MenuEntry> CountingFramePlugin::menuEntries() const
{
  return QList<MenuEntry>();
}

//------------------------------------------------------------------------
AnalysisReaderSList CountingFramePlugin::analysisReaders() const
{
  return AnalysisReaderSList();
}

//------------------------------------------------------------------------
void CountingFramePlugin::onAnalysisClosed()
{
  if(m_dockWidget != nullptr)
  {
    dynamic_cast<Panel *>(m_dockWidget)->deleteCountingFrames();
  }
}

Q_EXPORT_PLUGIN2(CountingFramePlugin, ESPINA::CF::CountingFramePlugin)
