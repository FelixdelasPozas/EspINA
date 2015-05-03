/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Widget2D.h"

#include "vtkWidget2D.h"
#include <GUI/View/RenderView.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;

class Widget2D::Command
: public vtkCommand
{
public:
  vtkTypeMacro(Command, vtkCommand);

  /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
   *
   */
  static Command *New()
  { return new Command(); }

  void setWidget(vtkWidget2D *widget);

  virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

private:
  explicit Command()
  : m_widget(nullptr)
  {}

  virtual ~Command() {};

private:
  vtkWidget2D *m_widget;
};

//----------------------------------------------------------------------------
void Widget2D::Command::setWidget(vtkWidget2D *widget)
{
  m_widget = widget;
}

//----------------------------------------------------------------------------
void Widget2D::Command::Execute(vtkObject *caller, long unsigned int eventId, void *callData)
{
  if(strcmp("vtkOpenGLCamera", caller->GetClassName()) == 0)
  {
    auto camera = dynamic_cast<vtkCamera *>(caller);
    Q_ASSERT(camera);

    m_widget->drawActors();
  }
}

//----------------------------------------------------------------------------
Widget2D::Widget2D(SelectionSPtr selection)
: m_index(0)
, m_selection(selection)
, m_widget(vtkSmartPointer<vtkWidget2D>::New())
, m_command(vtkSmartPointer<Command>::New())
, m_camera(nullptr)
, m_slice{0}
, m_selectedSegmentations(selection->segmentations())
{
}

//----------------------------------------------------------------------------
void Widget2D::setPlane(Plane plane)
{
  m_widget->setPlane(plane);

  m_index = normalCoordinateIndex(plane);
}

//----------------------------------------------------------------------------
void Widget2D::setRepresentationDepth(Nm depth)
{

}

//----------------------------------------------------------------------------
EspinaWidget2DSPtr Widget2D::clone()
{
  return std::make_shared<Widget2D>(m_selection);
}

//----------------------------------------------------------------------------
bool Widget2D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return true;
}

//----------------------------------------------------------------------------
bool Widget2D::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return true;
}

//----------------------------------------------------------------------------
void Widget2D::initializeImplementation(RenderView *view)
{
  m_camera = view->mainRenderer()->GetActiveCamera();

  m_camera->AddObserver(vtkCommand::ModifiedEvent, m_command);

  m_command->setWidget(m_widget);

  updateSelectionMeasure();

  synchronizeSelectionChanges();

  connect(m_selection.get(), SIGNAL(selectionChanged()),
          this,              SLOT(onSelectionChanged()));
}

//----------------------------------------------------------------------------
void Widget2D::uninitializeImplementation()
{
  m_camera->RemoveObserver(m_command);
  m_command->setWidget(nullptr);

  disconnect(m_selection.get(), SIGNAL(selectionChanged()),
             this,              SLOT(onSelectionChanged()));
  desynchronizeSelectionChanges();
}

//----------------------------------------------------------------------------
vtkAbstractWidget *Widget2D::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void Widget2D::setCrosshair(const NmVector3 &crosshair)
{
  m_slice = crosshair[m_index];

  updateVisibility();
}

//----------------------------------------------------------------------------
void Widget2D::synchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    connect(segmentation, SIGNAL(outputModified()),
            this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget2D::desynchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    disconnect(segmentation, SIGNAL(outputModified()),
               this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget2D::updateVisibility()
{
  auto bounds = m_widget->bounds();

  bool visible = bounds[2*m_index]  <= m_slice
              && bounds[2*m_index+1] > m_slice;

  m_widget->SetEnabled(visible);
}

//----------------------------------------------------------------------------
void Widget2D::onSelectionChanged()
{
  desynchronizeSelectionChanges();

  m_selectedSegmentations = m_selection->segmentations();

  synchronizeSelectionChanges();

  updateSelectionMeasure();
}

//----------------------------------------------------------------------------
void Widget2D::updateSelectionMeasure()
{
  Bounds selectionMeasure;

  for (auto segmentation : m_selectedSegmentations)
  {
    updateBoundingBox(selectionMeasure, segmentation->bounds());
  }

  if (!selectionMeasure.areValid())
  {
    for (auto channel : m_selection->channels())
    {
      updateBoundingBox(selectionMeasure, channel->bounds());
    }
  }

  m_widget->setBounds(selectionMeasure);

  updateVisibility();
}
