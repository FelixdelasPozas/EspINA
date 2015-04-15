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
void EspinaWidget::initialize(RenderView *view)
{
  auto widget     = vtkWidget();
  auto renderer   = view->mainRenderer();
  auto interactor = renderer->GetRenderWindow()->GetInteractor();

  widget->SetCurrentRenderer(renderer);
  widget->SetInteractor(interactor);

  initializeImplementation();
}

//-----------------------------------------------------------------------------
void EspinaWidget::uninitialize()
{
  auto widget = vtkWidget();

  widget->SetInteractor(nullptr);
  widget->SetCurrentRenderer(nullptr);

  uninitializeImplementation();
}

//-----------------------------------------------------------------------------
void EspinaWidget::show()
{
  auto widget = vtkWidget();

  widget->On();
}

//-----------------------------------------------------------------------------
void EspinaWidget::hide()
{
  auto widget = vtkWidget();

  widget->Off();
}

//-----------------------------------------------------------------------------
bool EspinaWidget::isEnabled()
{
  return vtkWidget()->GetEnabled();
}
