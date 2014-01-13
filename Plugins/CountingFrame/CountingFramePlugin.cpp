#include "CountingFramePlugin.h"

#include "Panel.h"
#include "CountingFrameRenderer.h"
#include "ColorEngines/CountingFrameColorEngine.h"

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
                               QUndoStack      *undoStack)
{
  m_model       = model;
  m_viewManager = viewManager;
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
QList<DockWidget *> CountingFramePlugin::dockWidgets()
{
  QList<DockWidget *> docks;

  docks << new Panel(&m_manager, m_model, m_viewManager);

  return docks;
}

//------------------------------------------------------------------------
RendererSList CountingFramePlugin::renderers()
{
  RendererSList renderers;

  renderers << RendererSPtr(new CountingFrameRenderer(m_manager));

  return renderers;
}

//------------------------------------------------------------------------
SettingsPanelSList CountingFramePlugin::settingsPanels()
{
  return SettingsPanelSList();
}



Q_EXPORT_PLUGIN2(CountingFramePlugin, EspINA::CF::CountingFramePlugin)