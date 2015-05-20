#include <GUI/View/Widgets/EspinaWidget2.h>
#include <GUI/View/RenderView.h>
#include <vtkAbstractWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
EspinaWidget::EspinaWidget()
{
}

//-----------------------------------------------------------------------------
EspinaWidget::~EspinaWidget()
{
}

//-----------------------------------------------------------------------------
void EspinaWidget::initializeWidget(RenderView *view)
{
  auto widget     = vtkWidget();
  auto renderer   = view->mainRenderer();
  auto interactor = renderer->GetRenderWindow()->GetInteractor();

  widget->SetCurrentRenderer(renderer);
  widget->SetInteractor(interactor);

  initializeImplementation(view);
}

//-----------------------------------------------------------------------------
void EspinaWidget::uninitializeWidget()
{
  auto widget = vtkWidget();

  widget->SetInteractor(nullptr);
  widget->SetCurrentRenderer(nullptr);

  uninitializeImplementation();
}

//-----------------------------------------------------------------------------
void EspinaWidget::showWidget()
{
  auto widget = vtkWidget();

  widget->On();
}

//-----------------------------------------------------------------------------
void EspinaWidget::hideWidget()
{
  auto widget = vtkWidget();

  widget->Off();
}

//-----------------------------------------------------------------------------
bool EspinaWidget::isWidgetEnabled()
{
  return vtkWidget()->GetEnabled();
}