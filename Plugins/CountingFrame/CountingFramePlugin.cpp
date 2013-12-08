#include "CountingFramePlugin.h"

#include "Panel.h"

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
QList<DockWidget *> CountingFramePlugin::dockWidgets()
{
  QList<DockWidget *> docks;

  docks << new Panel(&m_manager, m_model, m_viewManager);

  return docks;
}


Q_EXPORT_PLUGIN2(CountingFramePlugin, EspINA::CF::CountingFramePlugin)