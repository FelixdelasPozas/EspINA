#include "CountingFramePlugin.h"

#include "Panel.h"
#include "ColorEngines/ColorEngine.h"
#include "ColorEngines/ColorEngineSwitch.h"
#include "Extensions/CountingFrameFactories.h"
#include "Representations/RepresentationFactory.h"
#include <Support/Widgets/PanelSwitch.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
CountingFramePlugin::CountingFramePlugin()
: m_context                     {nullptr}
, m_scheduler                   {nullptr}
, m_undoStack                   {nullptr}
, m_manager                     {std::make_shared<CountingFrameManager>()}
, m_dockWidget                  {nullptr}
, m_colorEngine                 {nullptr}
, m_representationFactory       {nullptr}
, m_segmentationExtensionFactory{std::make_shared<CFSegmentationExtensionFactory>()}
, m_channelExtensionFactory     {std::make_shared<CFStackExtensionFactory>(m_segmentationExtensionFactory, m_manager.get())}
{
}

//------------------------------------------------------------------------
void CountingFramePlugin::init(Support::Context &context)
{
  if(m_context)
  {
    auto message = tr("Already initialized Counting Frame plugin!");
    auto details = tr("CountingFramePlugin::init(context) ->") + message;

    throw EspinaException(message, details);
  }

  m_context     = &context;
  m_scheduler   = context.scheduler();
  m_undoStack   = context.undoStack();

  m_manager->setContext(context);

  m_dockWidget            = new Panel(m_manager.get(), context, DefaultDialogs::defaultParentWidget());
  m_colorEngine           = std::make_shared<CF::ColorEngine>();
  m_representationFactory = std::make_shared<RepresentationFactory>(m_manager.get());

  std::dynamic_pointer_cast<CFStackExtensionFactory>(m_channelExtensionFactory)->setScheduler(m_scheduler);
}

//------------------------------------------------------------------------
StackExtensionFactorySList CountingFramePlugin::channelExtensionFactories() const
{
  if(m_channelExtensionFactory == nullptr)
  {
    auto message = tr("Counting frame plugin not initialized.");
    auto details = tr("CFPlugin::channelExtensionFactories() -> ") + message;

    throw EspinaException(message, details);
  }

  StackExtensionFactorySList factories;

  factories << m_channelExtensionFactory;

  return factories;
}

//------------------------------------------------------------------------
SegmentationExtensionFactorySList CountingFramePlugin::segmentationExtensionFactories() const
{
  if(m_segmentationExtensionFactory == nullptr)
  {
    auto message = tr("Counting frame plugin not initialized.");
    auto details = tr("CFPlugin::segmentationExtensionFactories() -> ") + message;

    throw EspinaException(message, details);
  }

  SegmentationExtensionFactorySList factories;

  factories << m_segmentationExtensionFactory;

  return factories;
}

//------------------------------------------------------------------------
ColorEngineSwitchSList CountingFramePlugin::colorEngines() const
{
  if(m_colorEngine == nullptr)
  {
    auto message = tr("Counting frame plugin not initialized.");
    auto details = tr("CFPlugin::colorEngines() -> ") + message;

    throw EspinaException(message, details);
  }

  ColorEngineSwitchSList engines;

  engines << std::make_shared<CF::ColorEngineSwitch>(m_manager.get(), m_colorEngine, *m_context);

  return engines;
}

//------------------------------------------------------------------------
RepresentationFactorySList CountingFramePlugin::representationFactories() const
{
  if(m_representationFactory == nullptr)
  {
    auto message = tr("Counting frame plugin not initialized.");
    auto details = tr("CFPlugin::representationFactories() -> ") + message;

    throw EspinaException(message, details);
  }

  RepresentationFactorySList factories;

  factories << m_representationFactory;

  return factories;
}

//------------------------------------------------------------------------
QList<CategorizedTool > CountingFramePlugin::tools() const
{
  if(m_dockWidget == nullptr)
  {
    auto message = tr("Counting frame plugin not initialized.");
    auto details = tr("CFPlugin::tools() -> ") + message;

    throw EspinaException(message, details);
  }

  QList<CategorizedTool> tools;

  auto tool = std::make_shared<PanelSwitch>("StereologicalCF", m_dockWidget, ":cf-switch2D.svg", tr("Stereological Counting Frame"), *m_context);

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

//------------------------------------------------------------------------
void CountingFramePlugin::init(SchedulerSPtr scheduler)
{
  std::dynamic_pointer_cast<CFStackExtensionFactory>(m_channelExtensionFactory)->setScheduler(scheduler);
}

Q_EXPORT_PLUGIN2(CountingFramePlugin, ESPINA::CF::CountingFramePlugin)
