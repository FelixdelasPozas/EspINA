#include "CountingFramePlugin.h"

#include "Panel.h"
#include "CountingFrameRenderer3D.h"
#include "CountingFrameRenderer2D.h"
#include "ColorEngines/CountingFrameColorEngine.h"
#include "Extensions/CountingFrameFactories.h"

using namespace EspINA;
using namespace EspINA::CF;

//------------------------------------------------------------------------
CountingFramePlugin::CountingFramePlugin()
: m_undoStack(nullptr)
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
}

//------------------------------------------------------------------------
NamedColorEngineSList CountingFramePlugin::colorEngines()
{
  NamedColorEngineSList engines;

  engines << NamedColorEngine("Counting Frame", ColorEngineSPtr(new CountingFrameColorEngine()));

  return engines;
}

//------------------------------------------------------------------------
QList<ToolGroup* > CountingFramePlugin::toolGroups()
{
  QList<ToolGroup *> tools;

  return tools;

}

//------------------------------------------------------------------------
QList<DockWidget *> CountingFramePlugin::dockWidgets()
{
  QList<DockWidget *> docks;

  docks << new Panel(&m_manager, m_model, m_viewManager, m_scheduler);

  return docks;
}

//------------------------------------------------------------------------
ChannelExtensionFactorySList CountingFramePlugin::channelExtensionFactories() const
{
  ChannelExtensionFactorySList factories;

  factories << ChannelExtensionFactorySPtr{new ChannelExtensionFactoryCF(const_cast<CountingFrameManager *>(&m_manager), m_scheduler)};

  return factories;
}

//------------------------------------------------------------------------
SegmentationExtensionFactorySList CountingFramePlugin::segmentationExtensionFactories() const
{
  SegmentationExtensionFactorySList factories;

  factories << SegmentationExtensionFactorySPtr{new SegmentationExtensionFactoryCF()};

  return factories;
}

//------------------------------------------------------------------------
RendererSList CountingFramePlugin::renderers()
{
  RendererSList renderers;

  renderers << RendererSPtr(new CountingFrameRenderer3D(m_manager));
  renderers << RendererSPtr(new CountingFrameRenderer2D(m_manager));

  return renderers;
}

//------------------------------------------------------------------------
SettingsPanelSList CountingFramePlugin::settingsPanels()
{
  return SettingsPanelSList();
}

Q_EXPORT_PLUGIN2(CountingFramePlugin, EspINA::CF::CountingFramePlugin)
