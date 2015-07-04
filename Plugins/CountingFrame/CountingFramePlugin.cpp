#include "CountingFramePlugin.h"

#include "Panel.h"
#include "ColorEngines/CountingFrameColorEngine.h"
#include "Extensions/CountingFrameFactories.h"
#include "Representations/RepresentationFactory.h"
#include <Support/Widgets/PanelSwitch.h>
#include <Support/Widgets/ColorEngineSwitch.h>

using namespace ESPINA;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFramePlugin::CountingFramePlugin()
: m_undoStack  {nullptr}
, m_dockWidget {nullptr}
, m_context    {nullptr}
, m_colorEngine{std::make_shared<CountingFrameColorEngine>()}
{
}

//------------------------------------------------------------------------
CountingFramePlugin::~CountingFramePlugin()
{
}

//------------------------------------------------------------------------
void CountingFramePlugin::init(Support::Context &context)
{
  m_context = &context;

  m_scheduler   = context.scheduler();
  m_undoStack   = context.undoStack();

  m_representationFactory = std::make_shared<RepresentationFactory>(m_manager);

  m_dockWidget = new Panel(&m_manager, context);

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
ColorEngineSwitchSList CountingFramePlugin::colorEngines() const
{
  ColorEngineSwitchSList engines;

  engines << std::make_shared<ColorEngineSwitch>(m_colorEngine, ":cf-switch2D.svg", *m_context);

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
QList<CategorizedTool > CountingFramePlugin::tools() const
{
  QList<CategorizedTool> tools;

  auto tool = std::make_shared<PanelSwitch>("CFPanel", m_dockWidget, ":cf-switch2D.svg", tr("Stereological Counting Frame"), *m_context);
  tool->setGroupWith("counting_frame");

  tools << CategorizedTool(ToolCategory::ANALYZE, tool);

  return tools;
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
