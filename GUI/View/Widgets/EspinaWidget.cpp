#include <GUI/View/RenderView.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <vtkAbstractWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
void EspinaWidget::initializeWidget(RenderView *view)
{
  if(!view) return;

  vtkRenderWindowInteractor *interactor = nullptr;

  auto widget   = vtkWidget();
  auto renderer = view->mainRenderer();

  if(renderer)
  {
    auto renderWindow = renderer->GetRenderWindow();

    if(renderWindow)
    {
      interactor = renderWindow->GetInteractor();
    }
  }

  widget->SetCurrentRenderer(renderer);
  widget->SetInteractor(interactor);

  initializeImplementation(view);

  widget->On();
}

//-----------------------------------------------------------------------------
void EspinaWidget::uninitializeWidget()
{
  auto widget = vtkWidget();

  widget->Off();
  widget->SetInteractor(nullptr);
  widget->SetCurrentRenderer(nullptr);

  uninitializeImplementation();
}

//-----------------------------------------------------------------------------
void EspinaWidget::showWidget()
{
  auto widget = vtkWidget();

  if(!widget->GetEnabled())
  {
    widget->On();
  }
}

//-----------------------------------------------------------------------------
void EspinaWidget::hideWidget()
{
  auto widget = vtkWidget();

  if(widget->GetEnabled())
  {
    widget->Off();
  }
}

//-----------------------------------------------------------------------------
bool EspinaWidget::isWidgetEnabled()
{
  return vtkWidget()->GetEnabled();
}
