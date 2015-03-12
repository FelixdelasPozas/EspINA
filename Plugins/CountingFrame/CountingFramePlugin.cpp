#include "CountingFramePlugin.h"

#include "Panel.h"
#include "ColorEngines/CountingFrameColorEngine.h"
#include "Extensions/CountingFrameFactories.h"

using namespace ESPINA;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFramePlugin::CountingFramePlugin()
: m_undoStack                   {nullptr}
, m_colorEngine                 {NamedColorEngine()}
, m_dockWidget                  {nullptr}
, m_channelExtensionFactory     {nullptr}
, m_segmentationExtensionFactory{nullptr}
// , m_renderer3d                  {nullptr}
// , m_renderer2d                  {nullptr}
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

  m_colorEngine = NamedColorEngine("Counting Frame", ColorEngineSPtr(new CountingFrameColorEngine()));
  m_dockWidget = new Panel(&m_manager, m_model, m_viewManager, m_scheduler);
  m_channelExtensionFactory = ChannelExtensionFactorySPtr{new ChannelExtensionFactoryCF(const_cast<CountingFrameManager *>(&m_manager), m_scheduler)};
  m_segmentationExtensionFactory = SegmentationExtensionFactorySPtr{new SegmentationExtensionFactoryCF()};
//   m_renderer3d = RendererSPtr(new CountingFrameRenderer3D(m_manager));
//   m_renderer2d = RendererSPtr(new CountingFrameRenderer2D(m_manager));
}

//------------------------------------------------------------------------
NamedColorEngineSList CountingFramePlugin::colorEngines() const
{
  NamedColorEngineSList engines;

  engines << m_colorEngine;

  return engines;
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
